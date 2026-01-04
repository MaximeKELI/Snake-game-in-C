#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ncurses.h>

#define WIDTH 60
#define HEIGHT 20
#define MAX_LENGTH 1000

// Structure pour une position
typedef struct {
    int x;
    int y;
} Position;

// Structure pour le serpent
typedef struct {
    Position body[MAX_LENGTH];
    int length;
    int direction;  // 0: haut, 1: droite, 2: bas, 3: gauche
    char head_char;
    char body_char;
} Snake;

// Structure pour le jeu
typedef struct {
    Snake snake;
    Position food;
    int score;
    int level;
    int speed;
    int game_over;
    int paused;
    int best_score;
    WINDOW *win;
} Game;

// Couleurs
#define COLOR_SNAKE_HEAD 1
#define COLOR_SNAKE_BODY 2
#define COLOR_FOOD 3
#define COLOR_BORDER 4
#define COLOR_TEXT 5

// Directions
#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3

// Fonction pour initialiser les couleurs
void init_colors() {
    start_color();
    init_pair(COLOR_SNAKE_HEAD, COLOR_GREEN, COLOR_BLACK);
    init_pair(COLOR_SNAKE_BODY, COLOR_YELLOW, COLOR_BLACK);
    init_pair(COLOR_FOOD, COLOR_RED, COLOR_BLACK);
    init_pair(COLOR_BORDER, COLOR_CYAN, COLOR_BLACK);
    init_pair(COLOR_TEXT, COLOR_WHITE, COLOR_BLACK);
}

// Fonction pour charger le meilleur score
int load_best_score() {
    FILE *file = fopen(".snake_best_score", "r");
    int score = 0;
    if (file) {
        fscanf(file, "%d", &score);
        fclose(file);
    }
    return score;
}

// Fonction pour sauvegarder le meilleur score
void save_best_score(int score) {
    FILE *file = fopen(".snake_best_score", "w");
    if (file) {
        fprintf(file, "%d", score);
        fclose(file);
    }
}

// Fonction pour initialiser le jeu
void init_game(Game *game) {
    game->win = newwin(HEIGHT + 2, WIDTH + 2, (LINES - HEIGHT) / 2 - 1, (COLS - WIDTH) / 2 - 1);
    keypad(game->win, TRUE);
    nodelay(game->win, TRUE);
    
    // Initialiser le serpent au centre
    game->snake.length = 3;
    game->snake.direction = RIGHT;
    game->snake.head_char = '@';
    game->snake.body_char = 'o';
    
    int start_x = WIDTH / 2;
    int start_y = HEIGHT / 2;
    
    for (int i = 0; i < game->snake.length; i++) {
        game->snake.body[i].x = start_x - i;
        game->snake.body[i].y = start_y;
    }
    
    // Position initiale de la nourriture
    game->food.x = WIDTH / 2 + 5;
    game->food.y = HEIGHT / 2;
    
    game->score = 0;
    game->level = 1;
    game->speed = 150;  // millisecondes
    game->game_over = 0;
    game->paused = 0;
    game->best_score = load_best_score();
}

// Fonction pour générer une nouvelle position de nourriture
void generate_food(Game *game) {
    int valid = 0;
    while (!valid) {
        game->food.x = rand() % WIDTH;
        game->food.y = rand() % HEIGHT;
        valid = 1;
        
        // Vérifier que la nourriture n'est pas sur le serpent
        for (int i = 0; i < game->snake.length; i++) {
            if (game->snake.body[i].x == game->food.x && 
                game->snake.body[i].y == game->food.y) {
                valid = 0;
                break;
            }
        }
    }
}

// Fonction pour déplacer le serpent
void move_snake(Game *game) {
    if (game->paused || game->game_over) return;
    
    Position head = game->snake.body[0];
    
    // Calculer la nouvelle position de la tête
    switch (game->snake.direction) {
        case UP:
            head.y--;
            break;
        case RIGHT:
            head.x++;
            break;
        case DOWN:
            head.y++;
            break;
        case LEFT:
            head.x--;
            break;
    }
    
    // Vérifier les collisions avec les murs
    if (head.x < 0 || head.x >= WIDTH || head.y < 0 || head.y >= HEIGHT) {
        game->game_over = 1;
        return;
    }
    
    // Vérifier les collisions avec le corps
    for (int i = 0; i < game->snake.length; i++) {
        if (head.x == game->snake.body[i].x && head.y == game->snake.body[i].y) {
            game->game_over = 1;
            return;
        }
    }
    
    // Déplacer le corps
    for (int i = game->snake.length; i > 0; i--) {
        game->snake.body[i] = game->snake.body[i - 1];
    }
    
    game->snake.body[0] = head;
    
    // Vérifier si le serpent mange la nourriture
    if (head.x == game->food.x && head.y == game->food.y) {
        game->snake.length++;
        game->score += 10;
        
        // Augmenter le niveau tous les 50 points
        int new_level = (game->score / 50) + 1;
        if (new_level > game->level) {
            game->level = new_level;
            game->speed = 150 - (game->level - 1) * 10;
            if (game->speed < 50) game->speed = 50;  // Vitesse minimum
        }
        
        generate_food(game);
    }
}

// Fonction pour gérer les entrées clavier
void handle_input(Game *game) {
    int ch = wgetch(game->win);
    
    switch (ch) {
        case KEY_UP:
        case 'w':
        case 'W':
            if (game->snake.direction != DOWN)
                game->snake.direction = UP;
            break;
        case KEY_RIGHT:
        case 'd':
        case 'D':
            if (game->snake.direction != LEFT)
                game->snake.direction = RIGHT;
            break;
        case KEY_DOWN:
        case 's':
        case 'S':
            if (game->snake.direction != UP)
                game->snake.direction = DOWN;
            break;
        case KEY_LEFT:
        case 'a':
        case 'A':
            if (game->snake.direction != RIGHT)
                game->snake.direction = LEFT;
            break;
        case 'p':
        case 'P':
            if (!game->game_over)
                game->paused = !game->paused;
            break;
        case 'q':
        case 'Q':
            game->game_over = 1;
            break;
    }
}

// Fonction pour dessiner le jeu
void draw_game(Game *game) {
    werase(game->win);
    box(game->win, 0, 0);
    
    // Dessiner la bordure avec couleur
    wattron(game->win, COLOR_PAIR(COLOR_BORDER));
    box(game->win, 0, 0);
    wattroff(game->win, COLOR_PAIR(COLOR_BORDER));
    
    // Dessiner le serpent
    for (int i = 0; i < game->snake.length; i++) {
        if (i == 0) {
            wattron(game->win, COLOR_PAIR(COLOR_SNAKE_HEAD));
            mvwaddch(game->win, game->snake.body[i].y + 1, game->snake.body[i].x + 1, game->snake.head_char);
            wattroff(game->win, COLOR_PAIR(COLOR_SNAKE_HEAD));
        } else {
            wattron(game->win, COLOR_PAIR(COLOR_SNAKE_BODY));
            mvwaddch(game->win, game->snake.body[i].y + 1, game->snake.body[i].x + 1, game->snake.body_char);
            wattroff(game->win, COLOR_PAIR(COLOR_SNAKE_BODY));
        }
    }
    
    // Dessiner la nourriture
    wattron(game->win, COLOR_PAIR(COLOR_FOOD));
    mvwaddch(game->win, game->food.y + 1, game->food.x + 1, '*');
    wattroff(game->win, COLOR_PAIR(COLOR_FOOD));
    
    // Afficher les informations
    wattron(game->win, COLOR_PAIR(COLOR_TEXT));
    char info[100];
    snprintf(info, sizeof(info), "Score: %d | Niveau: %d | Longueur: %d", 
             game->score, game->level, game->snake.length);
    mvwprintw(game->win, 0, 2, info);
    
    if (game->paused) {
        char pause_msg[] = "PAUSE - Appuyez sur P pour continuer";
        int x = (WIDTH - strlen(pause_msg)) / 2;
        mvwprintw(game->win, HEIGHT / 2, x, pause_msg);
    }
    
    wattroff(game->win, COLOR_PAIR(COLOR_TEXT));
    
    wrefresh(game->win);
}

// Fonction pour afficher le menu principal
int show_menu() {
    WINDOW *menu_win = newwin(15, 50, (LINES - 15) / 2, (COLS - 50) / 2);
    keypad(menu_win, TRUE);
    
    int selected = 0;
    const char *options[] = {
        "Commencer le jeu",
        "Quitter"
    };
    int num_options = 2;
    
    while (1) {
        werase(menu_win);
        box(menu_win, 0, 0);
        
        wattron(menu_win, COLOR_PAIR(COLOR_BORDER));
        box(menu_win, 0, 0);
        wattroff(menu_win, COLOR_PAIR(COLOR_BORDER));
        
        wattron(menu_win, COLOR_PAIR(COLOR_TEXT) | A_BOLD);
        mvwprintw(menu_win, 2, 15, "JEU DU SERPENT");
        wattroff(menu_win, COLOR_PAIR(COLOR_TEXT) | A_BOLD);
        
        // Charger et afficher le meilleur score
        int best = load_best_score();
        if (best > 0) {
            char best_str[50];
            snprintf(best_str, sizeof(best_str), "Meilleur score: %d", best);
            mvwprintw(menu_win, 4, 12, best_str);
        }
        
        for (int i = 0; i < num_options; i++) {
            if (i == selected) {
                wattron(menu_win, COLOR_PAIR(COLOR_SNAKE_HEAD) | A_BOLD);
                mvwprintw(menu_win, 7 + i * 2, 15, "> %s", options[i]);
                wattroff(menu_win, COLOR_PAIR(COLOR_SNAKE_HEAD) | A_BOLD);
            } else {
                wattron(menu_win, COLOR_PAIR(COLOR_TEXT));
                mvwprintw(menu_win, 7 + i * 2, 15, "  %s", options[i]);
                wattroff(menu_win, COLOR_PAIR(COLOR_TEXT));
            }
        }
        
        mvwprintw(menu_win, 12, 10, "Utilisez les fleches pour naviguer");
        mvwprintw(menu_win, 13, 12, "Appuyez sur ENTER pour selectionner");
        
        wrefresh(menu_win);
        
        int ch = wgetch(menu_win);
        switch (ch) {
            case KEY_UP:
                selected = (selected - 1 + num_options) % num_options;
                break;
            case KEY_DOWN:
                selected = (selected + 1) % num_options;
                break;
            case '\n':
            case KEY_ENTER:
                delwin(menu_win);
                return selected;
            case 'q':
            case 'Q':
                delwin(menu_win);
                return 1;
        }
    }
}

// Fonction pour afficher l'écran Game Over
int show_game_over(Game *game) {
    WINDOW *gameover_win = newwin(12, 50, (LINES - 12) / 2, (COLS - 50) / 2);
    keypad(gameover_win, TRUE);
    
    int selected = 0;
    const char *options[] = {
        "Rejouer",
        "Retour au menu",
        "Quitter"
    };
    int num_options = 3;
    
    // Mettre à jour le meilleur score
    if (game->score > game->best_score) {
        game->best_score = game->score;
        save_best_score(game->best_score);
    }
    
    while (1) {
        werase(gameover_win);
        box(gameover_win, 0, 0);
        
        wattron(gameover_win, COLOR_PAIR(COLOR_FOOD) | A_BOLD);
        mvwprintw(gameover_win, 2, 15, "GAME OVER!");
        wattroff(gameover_win, COLOR_PAIR(COLOR_FOOD) | A_BOLD);
        
        wattron(gameover_win, COLOR_PAIR(COLOR_TEXT));
        char score_str[50];
        snprintf(score_str, sizeof(score_str), "Score final: %d", game->score);
        mvwprintw(gameover_win, 4, 16, score_str);
        
        char best_str[50];
        snprintf(best_str, sizeof(best_str), "Meilleur score: %d", game->best_score);
        mvwprintw(gameover_win, 5, 15, best_str);
        
        for (int i = 0; i < num_options; i++) {
            if (i == selected) {
                wattron(gameover_win, COLOR_PAIR(COLOR_SNAKE_HEAD) | A_BOLD);
                mvwprintw(gameover_win, 7 + i, 17, "> %s", options[i]);
                wattroff(gameover_win, COLOR_PAIR(COLOR_SNAKE_HEAD) | A_BOLD);
            } else {
                mvwprintw(gameover_win, 7 + i, 17, "  %s", options[i]);
            }
        }
        
        wattroff(gameover_win, COLOR_PAIR(COLOR_TEXT));
        wrefresh(gameover_win);
        
        int ch = wgetch(gameover_win);
        switch (ch) {
            case KEY_UP:
                selected = (selected - 1 + num_options) % num_options;
                break;
            case KEY_DOWN:
                selected = (selected + 1) % num_options;
                break;
            case '\n':
            case KEY_ENTER:
                delwin(gameover_win);
                return selected;
            case 'q':
            case 'Q':
                delwin(gameover_win);
                return 2;
        }
    }
}

// Fonction principale du jeu
void game_loop(Game *game) {
    clock_t last_move = clock();
    
    while (!game->game_over) {
        handle_input(game);
        
        clock_t current = clock();
        double elapsed = ((double)(current - last_move) / CLOCKS_PER_SEC) * 1000;
        
        if (elapsed >= game->speed && !game->paused) {
            move_snake(game);
            last_move = current;
        }
        
        draw_game(game);
        usleep(10000);  // 10ms pour éviter de surcharger le CPU
    }
}

// Fonction principale
int main() {
    // Initialiser ncurses
    initscr();
    noecho();
    curs_set(0);
    init_colors();
    
    srand(time(NULL));
    
    int running = 1;
    
    while (running) {
        int menu_choice = show_menu();
        
        if (menu_choice == 0) {  // Commencer le jeu
            Game game;
            init_game(&game);
            generate_food(&game);
            
            game_loop(&game);
            
            int gameover_choice = show_game_over(&game);
            
            if (gameover_choice == 0) {  // Rejouer
                continue;
            } else if (gameover_choice == 1) {  // Retour au menu
                continue;
            } else {  // Quitter
                running = 0;
            }
        } else {  // Quitter
            running = 0;
        }
    }
    
    endwin();
    return 0;
}

