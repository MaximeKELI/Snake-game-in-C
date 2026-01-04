#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ncurses.h>

// ===== CONSTANTES =====
#define WIDTH 60
#define HEIGHT 20
#define MAX_LENGTH 1000
#define MAX_OBSTACLES 50
#define MAX_FOOD 5
#define MAX_TOP_SCORES 10
#define POWERUP_DURATION 100  // nombre de mouvements

// ===== ENUMS =====
typedef enum {
    FOOD_NORMAL = 0,
    FOOD_GOLDEN,
    FOOD_POISON,
    FOOD_FAST,
    FOOD_BONUS
} FoodType;

typedef enum {
    POWERUP_NONE = 0,
    POWERUP_SLOW,
    POWERUP_INVINCIBLE,
    POWERUP_MULTIPLIER,
    POWERUP_MAGNETIC
} PowerUpType;

typedef enum {
    MODE_CLASSIC = 0,
    MODE_ARCADE,
    MODE_CHALLENGE,
    MODE_FREE
} GameMode;

typedef enum {
    DIFF_EASY = 0,
    DIFF_MEDIUM,
    DIFF_HARD,
    DIFF_EXTREME
} Difficulty;

typedef enum {
    THEME_CLASSIC = 0,
    THEME_NEON,
    THEME_RETRO,
    THEME_DARK
} Theme;

typedef enum {
    UP = 0,
    RIGHT,
    DOWN,
    LEFT
} Direction;

// ===== STRUCTURES =====
typedef struct {
    int x;
    int y;
} Position;

typedef struct {
    Position pos;
    FoodType type;
    int timer;  // pour nourriture qui disparaît
    int pulse;  // animation
} Food;

typedef struct {
    Position pos;
    PowerUpType type;
    int timer;
    int active;
} PowerUp;

typedef struct {
    Position pos;
    int timer;  // pour obstacles animés
    int type;   // 0: fixe, 1: animé, 2: téléporteur
    Position portal_dest;  // pour téléporteurs
} Obstacle;

typedef struct {
    Position body[MAX_LENGTH];
    int length;
    Direction direction;
    char head_char;
    char body_char;
    int color_head;
    int color_body;
    int lives;  // pour mode arcade
    int score;
    int multiplier;
    int combo_count;
    time_t last_food_time;
} Snake;

typedef struct {
    int score;
    int level;
    char name[20];
    time_t date;
} TopScore;

typedef struct {
    // Serpents
    Snake snake1;
    Snake snake2;
    int multiplayer;
    
    // Nourriture
    Food foods[MAX_FOOD];
    int food_count;
    
    // Power-ups
    PowerUp powerup;
    
    // Obstacles
    Obstacle obstacles[MAX_OBSTACLES];
    int obstacle_count;
    
    // État du jeu
    int score;
    int level;
    int speed;
    int base_speed;
    int game_over;
    int paused;
    int winner;  // pour multijoueur
    
    // Configuration
    GameMode mode;
    Difficulty difficulty;
    Theme theme;
    int grid_width;
    int grid_height;
    
    // Statistiques
    int food_eaten;
    int time_played;
    time_t start_time;
    
    // Effets
    int slow_timer;
    int invincible_timer;
    int multiplier_timer;
    int magnetic_timer;
    
    // Fenêtre
    WINDOW *win;
    
    // Top scores
    TopScore top_scores[MAX_TOP_SCORES];
    int top_score_count;
} Game;

// ===== COULEURS =====
#define COLOR_SNAKE1_HEAD 1
#define COLOR_SNAKE1_BODY 2
#define COLOR_SNAKE2_HEAD 3
#define COLOR_SNAKE2_BODY 4
#define COLOR_FOOD_NORMAL 5
#define COLOR_FOOD_GOLDEN 6
#define COLOR_FOOD_POISON 7
#define COLOR_FOOD_FAST 8
#define COLOR_FOOD_BONUS 9
#define COLOR_POWERUP 10
#define COLOR_OBSTACLE 11
#define COLOR_BORDER 12
#define COLOR_TEXT 13
#define COLOR_PORTAL 14

// ===== PROTOTYPES =====
void init_colors();
void init_theme_colors(Theme theme);
void init_game(Game *game, GameMode mode, Difficulty diff, int multiplayer);
void init_snake(Snake *snake, int start_x, int start_y, int player_num);
Position generate_random_position(Game *game);
int is_position_valid(Game *game, Position pos, int check_snake);
void generate_food(Game *game);
void generate_powerup(Game *game);
void generate_obstacles(Game *game);
void move_snake(Game *game, Snake *snake);
void handle_input(Game *game);
void handle_input_multiplayer(Game *game);
void draw_game(Game *game);
void update_powerups(Game *game);
void check_food_collision(Game *game, Snake *snake);
void check_obstacle_collision(Game *game, Snake *snake);
int show_main_menu();
int show_game_mode_menu();
int show_difficulty_menu();
int show_theme_menu();
void show_top_scores(Game *game);
int show_game_over(Game *game);
void game_loop(Game *game);
void load_top_scores(Game *game);
void save_top_scores(Game *game);
void add_top_score(Game *game, int score);
char* get_food_char(FoodType type);
char* get_powerup_char(PowerUpType type);

// ===== IMPLÉMENTATION =====
void init_colors() {
    if (!has_colors()) return;
    start_color();
    use_default_colors();
}

void init_theme_colors(Theme theme) {
    if (!has_colors()) return;
    
    switch (theme) {
        case THEME_CLASSIC:
            init_pair(COLOR_SNAKE1_HEAD, COLOR_GREEN, COLOR_BLACK);
            init_pair(COLOR_SNAKE1_BODY, COLOR_YELLOW, COLOR_BLACK);
            init_pair(COLOR_SNAKE2_HEAD, COLOR_CYAN, COLOR_BLACK);
            init_pair(COLOR_SNAKE2_BODY, COLOR_BLUE, COLOR_BLACK);
            init_pair(COLOR_FOOD_NORMAL, COLOR_RED, COLOR_BLACK);
            init_pair(COLOR_FOOD_GOLDEN, COLOR_YELLOW, COLOR_BLACK);
            init_pair(COLOR_FOOD_POISON, COLOR_MAGENTA, COLOR_BLACK);
            init_pair(COLOR_FOOD_FAST, COLOR_CYAN, COLOR_BLACK);
            init_pair(COLOR_FOOD_BONUS, COLOR_WHITE, COLOR_BLACK);
            init_pair(COLOR_POWERUP, COLOR_GREEN, COLOR_BLACK);
            init_pair(COLOR_OBSTACLE, COLOR_WHITE, COLOR_BLACK);
            init_pair(COLOR_BORDER, COLOR_CYAN, COLOR_BLACK);
            init_pair(COLOR_TEXT, COLOR_WHITE, COLOR_BLACK);
            init_pair(COLOR_PORTAL, COLOR_MAGENTA, COLOR_BLACK);
            break;
        case THEME_NEON:
            init_pair(COLOR_SNAKE1_HEAD, COLOR_GREEN, COLOR_BLACK);
            init_pair(COLOR_SNAKE1_BODY, COLOR_GREEN, COLOR_BLACK);
            init_pair(COLOR_SNAKE2_HEAD, COLOR_CYAN, COLOR_BLACK);
            init_pair(COLOR_SNAKE2_BODY, COLOR_CYAN, COLOR_BLACK);
            init_pair(COLOR_FOOD_NORMAL, COLOR_RED, COLOR_BLACK);
            init_pair(COLOR_FOOD_GOLDEN, COLOR_YELLOW, COLOR_BLACK);
            init_pair(COLOR_FOOD_POISON, COLOR_MAGENTA, COLOR_BLACK);
            init_pair(COLOR_FOOD_FAST, COLOR_BLUE, COLOR_BLACK);
            init_pair(COLOR_FOOD_BONUS, COLOR_WHITE, COLOR_BLACK);
            init_pair(COLOR_POWERUP, COLOR_GREEN, COLOR_BLACK);
            init_pair(COLOR_OBSTACLE, COLOR_WHITE, COLOR_BLACK);
            init_pair(COLOR_BORDER, COLOR_CYAN, COLOR_BLACK);
            init_pair(COLOR_TEXT, COLOR_WHITE, COLOR_BLACK);
            init_pair(COLOR_PORTAL, COLOR_MAGENTA, COLOR_BLACK);
            break;
        case THEME_RETRO:
            init_pair(COLOR_SNAKE1_HEAD, COLOR_GREEN, COLOR_BLACK);
            init_pair(COLOR_SNAKE1_BODY, COLOR_GREEN, COLOR_BLACK);
            init_pair(COLOR_SNAKE2_HEAD, COLOR_YELLOW, COLOR_BLACK);
            init_pair(COLOR_SNAKE2_BODY, COLOR_YELLOW, COLOR_BLACK);
            init_pair(COLOR_FOOD_NORMAL, COLOR_RED, COLOR_BLACK);
            init_pair(COLOR_FOOD_GOLDEN, COLOR_YELLOW, COLOR_BLACK);
            init_pair(COLOR_FOOD_POISON, COLOR_MAGENTA, COLOR_BLACK);
            init_pair(COLOR_FOOD_FAST, COLOR_CYAN, COLOR_BLACK);
            init_pair(COLOR_FOOD_BONUS, COLOR_WHITE, COLOR_BLACK);
            init_pair(COLOR_POWERUP, COLOR_GREEN, COLOR_BLACK);
            init_pair(COLOR_OBSTACLE, COLOR_WHITE, COLOR_BLACK);
            init_pair(COLOR_BORDER, COLOR_WHITE, COLOR_BLACK);
            init_pair(COLOR_TEXT, COLOR_WHITE, COLOR_BLACK);
            init_pair(COLOR_PORTAL, COLOR_MAGENTA, COLOR_BLACK);
            break;
        case THEME_DARK:
            init_pair(COLOR_SNAKE1_HEAD, COLOR_GREEN, COLOR_BLACK);
            init_pair(COLOR_SNAKE1_BODY, COLOR_GREEN, COLOR_BLACK);
            init_pair(COLOR_SNAKE2_HEAD, COLOR_BLUE, COLOR_BLACK);
            init_pair(COLOR_SNAKE2_BODY, COLOR_BLUE, COLOR_BLACK);
            init_pair(COLOR_FOOD_NORMAL, COLOR_RED, COLOR_BLACK);
            init_pair(COLOR_FOOD_GOLDEN, COLOR_YELLOW, COLOR_BLACK);
            init_pair(COLOR_FOOD_POISON, COLOR_MAGENTA, COLOR_BLACK);
            init_pair(COLOR_FOOD_FAST, COLOR_CYAN, COLOR_BLACK);
            init_pair(COLOR_FOOD_BONUS, COLOR_WHITE, COLOR_BLACK);
            init_pair(COLOR_POWERUP, COLOR_GREEN, COLOR_BLACK);
            init_pair(COLOR_OBSTACLE, COLOR_WHITE, COLOR_BLACK);
            init_pair(COLOR_BORDER, COLOR_WHITE, COLOR_BLACK);
            init_pair(COLOR_TEXT, COLOR_WHITE, COLOR_BLACK);
            init_pair(COLOR_PORTAL, COLOR_MAGENTA, COLOR_BLACK);
            break;
    }
}

Position generate_random_position(Game *game) {
    Position pos;
    pos.x = rand() % game->grid_width;
    pos.y = rand() % game->grid_height;
    return pos;
}

int is_position_valid(Game *game, Position pos, int check_snake) {
    if (pos.x < 0 || pos.x >= game->grid_width || pos.y < 0 || pos.y >= game->grid_height)
        return 0;
    
    // Vérifier obstacles
    for (int i = 0; i < game->obstacle_count; i++) {
        if (game->obstacles[i].pos.x == pos.x && game->obstacles[i].pos.y == pos.y)
            return 0;
    }
    
    // Vérifier nourriture
    for (int i = 0; i < game->food_count; i++) {
        if (game->foods[i].pos.x == pos.x && game->foods[i].pos.y == pos.y)
            return 0;
    }
    
    // Vérifier serpents
    if (check_snake) {
        for (int i = 0; i < game->snake1.length; i++) {
            if (game->snake1.body[i].x == pos.x && game->snake1.body[i].y == pos.y)
                return 0;
        }
        if (game->multiplayer) {
            for (int i = 0; i < game->snake2.length; i++) {
                if (game->snake2.body[i].x == pos.x && game->snake2.body[i].y == pos.y)
                    return 0;
            }
        }
    }
    
    return 1;
}

void init_snake(Snake *snake, int start_x, int start_y, int player_num) {
    snake->length = 3;
    snake->direction = RIGHT;
    snake->lives = (player_num == 1) ? 3 : 3;
    snake->score = 0;
    snake->multiplier = 1;
    snake->combo_count = 0;
    snake->last_food_time = time(NULL);
    
    if (player_num == 1) {
        snake->head_char = '@';
        snake->body_char = 'o';
        snake->color_head = COLOR_SNAKE1_HEAD;
        snake->color_body = COLOR_SNAKE1_BODY;
    } else {
        snake->head_char = '#';
        snake->body_char = '*';
        snake->color_head = COLOR_SNAKE2_HEAD;
        snake->color_body = COLOR_SNAKE2_BODY;
    }
    
    for (int i = 0; i < snake->length; i++) {
        snake->body[i].x = start_x - i;
        snake->body[i].y = start_y;
    }
}

void init_game(Game *game, GameMode mode, Difficulty diff, int multiplayer) {
    game->mode = mode;
    game->difficulty = diff;
    game->multiplayer = multiplayer;
    game->theme = THEME_CLASSIC;
    
    // Configuration selon difficulté
    switch (diff) {
        case DIFF_EASY:
            game->grid_width = 80;
            game->grid_height = 30;
            game->base_speed = 200;
            break;
        case DIFF_MEDIUM:
            game->grid_width = 60;
            game->grid_height = 20;
            game->base_speed = 150;
            break;
        case DIFF_HARD:
            game->grid_width = 50;
            game->grid_height = 18;
            game->base_speed = 100;
            break;
        case DIFF_EXTREME:
            game->grid_width = 40;
            game->grid_height = 15;
            game->base_speed = 50;
            break;
    }
    
    game->win = newwin(game->grid_height + 2, game->grid_width + 2,
                       (LINES - game->grid_height) / 2 - 1,
                       (COLS - game->grid_width) / 2 - 1);
    keypad(game->win, TRUE);
    nodelay(game->win, TRUE);
    
    // Initialiser serpents
    int start_x = game->grid_width / 2;
    int start_y = game->grid_height / 2;
    init_snake(&game->snake1, start_x, start_y, 1);
    
    if (multiplayer) {
        init_snake(&game->snake2, start_x - 10, start_y, 2);
    }
    
    // Initialiser état
    game->score = 0;
    game->level = 1;
    game->speed = game->base_speed;
    game->game_over = 0;
    game->paused = 0;
    game->winner = 0;
    game->food_count = 1;
    game->obstacle_count = 0;
    game->food_eaten = 0;
    game->start_time = time(NULL);
    
    // Power-ups
    game->powerup.active = 0;
    game->slow_timer = 0;
    game->invincible_timer = 0;
    game->multiplier_timer = 0;
    game->magnetic_timer = 0;
    
    // Générer nourriture et obstacles
    generate_food(game);
    if (mode == MODE_CHALLENGE) {
        generate_obstacles(game);
    }
    
    load_top_scores(game);
}

void generate_food(Game *game) {
    for (int i = 0; i < game->food_count; i++) {
        Position pos;
        int attempts = 0;
        do {
            pos = generate_random_position(game);
            attempts++;
        } while (!is_position_valid(game, pos, 1) && attempts < 100);
        
        if (attempts < 100) {
            game->foods[i].pos = pos;
            
            // Choisir type de nourriture
            int r = rand() % 100;
            if (r < 50) {
                game->foods[i].type = FOOD_NORMAL;
            } else if (r < 70) {
                game->foods[i].type = FOOD_GOLDEN;
            } else if (r < 85) {
                game->foods[i].type = FOOD_POISON;
            } else if (r < 95) {
                game->foods[i].type = FOOD_FAST;
            } else {
                game->foods[i].type = FOOD_BONUS;
            }
            
            game->foods[i].timer = 0;
            game->foods[i].pulse = 0;
        }
    }
}

void generate_powerup(Game *game) {
    if (game->powerup.active) return;
    
    if (rand() % 100 < 15) {  // 15% chance
        Position pos;
        int attempts = 0;
        do {
            pos = generate_random_position(game);
            attempts++;
        } while (!is_position_valid(game, pos, 1) && attempts < 100);
        
        if (attempts < 100) {
            game->powerup.pos = pos;
            game->powerup.active = 1;
            game->powerup.timer = 0;
            
            int r = rand() % 4;
            switch (r) {
                case 0: game->powerup.type = POWERUP_SLOW; break;
                case 1: game->powerup.type = POWERUP_INVINCIBLE; break;
                case 2: game->powerup.type = POWERUP_MULTIPLIER; break;
                case 3: game->powerup.type = POWERUP_MAGNETIC; break;
            }
        }
    }
}

void generate_obstacles(Game *game) {
    int count = (game->grid_width * game->grid_height) / 50;
    if (count > MAX_OBSTACLES) count = MAX_OBSTACLES;
    
    game->obstacle_count = 0;
    for (int i = 0; i < count && game->obstacle_count < MAX_OBSTACLES; i++) {
        Position pos;
        int attempts = 0;
        do {
            pos = generate_random_position(game);
            attempts++;
        } while (!is_position_valid(game, pos, 0) && attempts < 100);
        
        if (attempts < 100) {
            game->obstacles[game->obstacle_count].pos = pos;
            game->obstacles[game->obstacle_count].timer = 0;
            game->obstacles[game->obstacle_count].type = (rand() % 10 < 2) ? 2 : 0;  // 20% téléporteurs
            
            if (game->obstacles[game->obstacle_count].type == 2) {
                Position dest;
                int dest_attempts = 0;
                do {
                    dest = generate_random_position(game);
                    dest_attempts++;
                } while ((dest.x == pos.x && dest.y == pos.y) && dest_attempts < 50);
                game->obstacles[game->obstacle_count].portal_dest = dest;
            }
            
            game->obstacle_count++;
        }
    }
}

void move_snake(Game *game, Snake *snake) {
    if (game->paused || game->game_over) return;
    
    Position head = snake->body[0];
    
    switch (snake->direction) {
        case UP: head.y--; break;
        case RIGHT: head.x++; break;
        case DOWN: head.y++; break;
        case LEFT: head.x--; break;
    }
    
    // Mode libre : passage à travers les murs
    if (game->mode == MODE_FREE) {
        if (head.x < 0) head.x = game->grid_width - 1;
        if (head.x >= game->grid_width) head.x = 0;
        if (head.y < 0) head.y = game->grid_height - 1;
        if (head.y >= game->grid_height) head.y = 0;
    } else {
        // Collision avec murs
        if (head.x < 0 || head.x >= game->grid_width ||
            head.y < 0 || head.y >= game->grid_height) {
            if (game->mode == MODE_ARCADE && snake->lives > 0) {
                snake->lives--;
                // Réinitialiser le serpent
                int start_x = game->grid_width / 2;
                int start_y = game->grid_height / 2;
                snake->length = 3;
                for (int j = 0; j < snake->length; j++) {
                    snake->body[j].x = start_x - j;
                    snake->body[j].y = start_y;
                }
                head = snake->body[0];
            } else {
                if (game->multiplayer) {
                    game->winner = (snake == &game->snake1) ? 2 : 1;
                }
                game->game_over = 1;
                return;
            }
        }
    }
    
    // Collision avec corps (sauf si invincible)
    if (game->invincible_timer == 0) {
        for (int i = 1; i < snake->length; i++) {
            if (head.x == snake->body[i].x && head.y == snake->body[i].y) {
                if (game->mode == MODE_ARCADE && snake->lives > 0) {
                    snake->lives--;
                    // Réinitialiser le serpent
                    int start_x = game->grid_width / 2;
                    int start_y = game->grid_height / 2;
                    snake->length = 3;
                    for (int j = 0; j < snake->length; j++) {
                        snake->body[j].x = start_x - j;
                        snake->body[j].y = start_y;
                    }
                    head = snake->body[0];
                } else {
                    if (game->multiplayer) {
                        game->winner = (snake == &game->snake1) ? 2 : 1;
                    }
                    game->game_over = 1;
                    return;
                }
            }
        }
    }
    
    // Collision avec autre serpent (multijoueur)
    if (game->multiplayer) {
        Snake *other = (snake == &game->snake1) ? &game->snake2 : &game->snake1;
        for (int i = 0; i < other->length; i++) {
            if (head.x == other->body[i].x && head.y == other->body[i].y) {
                if (game->invincible_timer == 0) {
                    if (game->mode == MODE_ARCADE && snake->lives > 0) {
                        snake->lives--;
                        // Réinitialiser le serpent
                        int start_x = game->grid_width / 2;
                        int start_y = game->grid_height / 2;
                        snake->length = 3;
                        for (int j = 0; j < snake->length; j++) {
                            snake->body[j].x = start_x - j;
                            snake->body[j].y = start_y;
                        }
                        head = snake->body[0];
                    } else {
                        game->winner = (snake == &game->snake1) ? 2 : 1;
                        game->game_over = 1;
                        return;
                    }
                }
            }
        }
    }
    
    // Déplacer corps
    for (int i = snake->length; i > 0; i--) {
        snake->body[i] = snake->body[i - 1];
    }
    snake->body[0] = head;
    
    // Vérifier collisions obstacles
    check_obstacle_collision(game, snake);
    
    // Vérifier collisions nourriture
    check_food_collision(game, snake);
    
    // Vérifier power-ups
    if (game->powerup.active) {
        if (head.x == game->powerup.pos.x && head.y == game->powerup.pos.y) {
            switch (game->powerup.type) {
                case POWERUP_SLOW:
                    game->slow_timer = POWERUP_DURATION;
                    game->speed = game->base_speed * 2;
                    break;
                case POWERUP_INVINCIBLE:
                    game->invincible_timer = POWERUP_DURATION;
                    break;
                case POWERUP_MULTIPLIER:
                    game->multiplier_timer = POWERUP_DURATION;
                    snake->multiplier = 2;
                    break;
                case POWERUP_MAGNETIC:
                    game->magnetic_timer = POWERUP_DURATION;
                    break;
                case POWERUP_NONE:
                default:
                    break;
            }
            game->powerup.active = 0;
        }
    }
}

void check_obstacle_collision(Game *game, Snake *snake) {
    Position head = snake->body[0];
    
    for (int i = 0; i < game->obstacle_count; i++) {
        if (head.x == game->obstacles[i].pos.x &&
            head.y == game->obstacles[i].pos.y) {
            if (game->obstacles[i].type == 2) {
                // Téléporteur
                snake->body[0] = game->obstacles[i].portal_dest;
            } else {
                // Obstacle normal
                if (game->invincible_timer == 0) {
                    if (game->mode == MODE_ARCADE && snake->lives > 0) {
                        snake->lives--;
                        snake->body[0].x = game->grid_width / 2;
                        snake->body[0].y = game->grid_height / 2;
                        snake->length = 3;
                    } else {
                        if (game->multiplayer) {
                            game->winner = (snake == &game->snake1) ? 2 : 1;
                        }
                        game->game_over = 1;
                    }
                }
            }
        }
    }
}

void check_food_collision(Game *game, Snake *snake) {
    Position head = snake->body[0];
    
    for (int i = 0; i < game->food_count; i++) {
        if (head.x == game->foods[i].pos.x && head.y == game->foods[i].pos.y) {
            int points = 0;
            int should_grow = 1;
            
            switch (game->foods[i].type) {
                case FOOD_NORMAL:
                    points = 10;
                    break;
                case FOOD_GOLDEN:
                    points = 50;
                    break;
                case FOOD_POISON:
                    if (snake->length > 3) {
                        snake->length -= 2;
                        should_grow = 0;
                    }
                    points = -5;
                    break;
                case FOOD_FAST:
                    game->speed = game->base_speed / 2;
                    points = 15;
                    break;
                case FOOD_BONUS:
                    points = 100;
                    break;
            }
            
            // Combo system
            time_t now = time(NULL);
            if (now - snake->last_food_time < 2) {
                snake->combo_count++;
                points = (int)(points * (1.0 + snake->combo_count * 0.1));
            } else {
                snake->combo_count = 0;
            }
            snake->last_food_time = now;
            
            points *= snake->multiplier;
            
            if (should_grow) {
                snake->length++;
                if (snake->length >= MAX_LENGTH) snake->length = MAX_LENGTH - 1;
            }
            
            snake->score += points;
            game->score += points;
            game->food_eaten++;
            
            // Augmenter niveau
            int new_level = (game->score / 100) + 1;
            if (new_level > game->level) {
                game->level = new_level;
                if (game->slow_timer == 0) {
                    game->speed = game->base_speed - (game->level - 1) * 5;
                    if (game->speed < 30) game->speed = 30;
                }
            }
            
            // Régénérer nourriture
            Position pos;
            int attempts = 0;
            do {
                pos = generate_random_position(game);
                attempts++;
            } while (!is_position_valid(game, pos, 1) && attempts < 100);
            
            if (attempts < 100) {
                game->foods[i].pos = pos;
                int r = rand() % 100;
                if (r < 50) game->foods[i].type = FOOD_NORMAL;
                else if (r < 70) game->foods[i].type = FOOD_GOLDEN;
                else if (r < 85) game->foods[i].type = FOOD_POISON;
                else if (r < 95) game->foods[i].type = FOOD_FAST;
                else game->foods[i].type = FOOD_BONUS;
            }
            
            // Chance de générer power-up
            generate_powerup(game);
            break;
        }
    }
}

void update_powerups(Game *game) {
    if (game->slow_timer > 0) {
        game->slow_timer--;
        if (game->slow_timer == 0) {
            game->speed = game->base_speed - (game->level - 1) * 5;
            if (game->speed < 30) game->speed = 30;
        }
    }
    
    if (game->invincible_timer > 0) {
        game->invincible_timer--;
    }
    
    if (game->multiplier_timer > 0) {
        game->multiplier_timer--;
        if (game->multiplier_timer == 0) {
            game->snake1.multiplier = 1;
            if (game->multiplayer) game->snake2.multiplier = 1;
        }
    }
    
    if (game->magnetic_timer > 0) {
        game->magnetic_timer--;
        // Attirer la nourriture vers le serpent (simplifié)
    }
    
    // Mettre à jour nourriture (timer)
    for (int i = 0; i < game->food_count; i++) {
        game->foods[i].timer++;
        game->foods[i].pulse = (game->foods[i].pulse + 1) % 10;
    }
}

void handle_input(Game *game) {
    int ch = wgetch(game->win);
    
    switch (ch) {
        case KEY_UP:
        case 'w':
        case 'W':
            if (game->snake1.direction != DOWN)
                game->snake1.direction = UP;
            break;
        case KEY_RIGHT:
        case 'd':
        case 'D':
            if (game->snake1.direction != LEFT)
                game->snake1.direction = RIGHT;
            break;
        case KEY_DOWN:
        case 's':
        case 'S':
            if (game->snake1.direction != UP)
                game->snake1.direction = DOWN;
            break;
        case KEY_LEFT:
        case 'a':
        case 'A':
            if (game->snake1.direction != RIGHT)
                game->snake1.direction = LEFT;
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

void handle_input_multiplayer(Game *game) {
    int ch = wgetch(game->win);
    
    // Joueur 1 (WASD)
    switch (ch) {
        case 'w':
        case 'W':
            if (game->snake1.direction != DOWN)
                game->snake1.direction = UP;
            break;
        case 'd':
        case 'D':
            if (game->snake1.direction != LEFT)
                game->snake1.direction = RIGHT;
            break;
        case 's':
        case 'S':
            if (game->snake1.direction != UP)
                game->snake1.direction = DOWN;
            break;
        case 'a':
        case 'A':
            if (game->snake1.direction != RIGHT)
                game->snake1.direction = LEFT;
            break;
    }
    
    // Joueur 2 (Flèches)
    switch (ch) {
        case KEY_UP:
            if (game->snake2.direction != DOWN)
                game->snake2.direction = UP;
            break;
        case KEY_RIGHT:
            if (game->snake2.direction != LEFT)
                game->snake2.direction = RIGHT;
            break;
        case KEY_DOWN:
            if (game->snake2.direction != UP)
                game->snake2.direction = DOWN;
            break;
        case KEY_LEFT:
            if (game->snake2.direction != RIGHT)
                game->snake2.direction = LEFT;
            break;
    }
    
    if (ch == 'p' || ch == 'P') {
        if (!game->game_over)
            game->paused = !game->paused;
    }
    if (ch == 'q' || ch == 'Q') {
        game->game_over = 1;
    }
}

char* get_food_char(FoodType type) {
    switch (type) {
        case FOOD_NORMAL: return "*";
        case FOOD_GOLDEN: return "$";
        case FOOD_POISON: return "X";
        case FOOD_FAST: return "!";
        case FOOD_BONUS: return "?";
        default: return "*";
    }
}

char* get_powerup_char(PowerUpType type) {
    switch (type) {
        case POWERUP_SLOW: return "S";
        case POWERUP_INVINCIBLE: return "I";
        case POWERUP_MULTIPLIER: return "M";
        case POWERUP_MAGNETIC: return "G";
        default: return "P";
    }
}

void draw_game(Game *game) {
    werase(game->win);
    box(game->win, 0, 0);
    
    wattron(game->win, COLOR_PAIR(COLOR_BORDER));
    box(game->win, 0, 0);
    wattroff(game->win, COLOR_PAIR(COLOR_BORDER));
    
    // Dessiner obstacles
    for (int i = 0; i < game->obstacle_count; i++) {
        int color = (game->obstacles[i].type == 2) ? COLOR_PORTAL : COLOR_OBSTACLE;
        wattron(game->win, COLOR_PAIR(color));
        char ch = (game->obstacles[i].type == 2) ? 'O' : '#';
        mvwaddch(game->win, game->obstacles[i].pos.y + 1,
                 game->obstacles[i].pos.x + 1, ch);
        wattroff(game->win, COLOR_PAIR(color));
    }
    
    // Dessiner nourriture
    for (int i = 0; i < game->food_count; i++) {
        int color_pair;
        switch (game->foods[i].type) {
            case FOOD_NORMAL: color_pair = COLOR_FOOD_NORMAL; break;
            case FOOD_GOLDEN: color_pair = COLOR_FOOD_GOLDEN; break;
            case FOOD_POISON: color_pair = COLOR_FOOD_POISON; break;
            case FOOD_FAST: color_pair = COLOR_FOOD_FAST; break;
            case FOOD_BONUS: color_pair = COLOR_FOOD_BONUS; break;
            default: color_pair = COLOR_FOOD_NORMAL; break;
        }
        
        wattron(game->win, COLOR_PAIR(color_pair));
        if (game->foods[i].pulse < 5) {
            wattron(game->win, A_BOLD);
        }
        mvwprintw(game->win, game->foods[i].pos.y + 1,
                  game->foods[i].pos.x + 1, "%s", get_food_char(game->foods[i].type));
        wattroff(game->win, A_BOLD);
        wattroff(game->win, COLOR_PAIR(color_pair));
    }
    
    // Dessiner power-up
    if (game->powerup.active) {
        wattron(game->win, COLOR_PAIR(COLOR_POWERUP) | A_BOLD);
        mvwprintw(game->win, game->powerup.pos.y + 1,
                  game->powerup.pos.x + 1, "%s", get_powerup_char(game->powerup.type));
        wattroff(game->win, A_BOLD);
        wattroff(game->win, COLOR_PAIR(COLOR_POWERUP));
    }
    
    // Dessiner serpent 1
    for (int i = 0; i < game->snake1.length; i++) {
        int color = (i == 0) ? game->snake1.color_head : game->snake1.color_body;
        wattron(game->win, COLOR_PAIR(color));
        if (i == 0 && game->invincible_timer > 0) {
            wattron(game->win, A_BOLD | A_BLINK);
        }
        char ch = (i == 0) ? game->snake1.head_char : game->snake1.body_char;
        mvwaddch(game->win, game->snake1.body[i].y + 1,
                 game->snake1.body[i].x + 1, ch);
        if (i == 0 && game->invincible_timer > 0) {
            wattroff(game->win, A_BOLD | A_BLINK);
        }
        wattroff(game->win, COLOR_PAIR(color));
    }
    
    // Dessiner serpent 2 (multijoueur)
    if (game->multiplayer) {
        for (int i = 0; i < game->snake2.length; i++) {
            int color = (i == 0) ? game->snake2.color_head : game->snake2.color_body;
            wattron(game->win, COLOR_PAIR(color));
            char ch = (i == 0) ? game->snake2.head_char : game->snake2.body_char;
            mvwaddch(game->win, game->snake2.body[i].y + 1,
                     game->snake2.body[i].x + 1, ch);
            wattroff(game->win, COLOR_PAIR(color));
        }
    }
    
    // Informations
    wattron(game->win, COLOR_PAIR(COLOR_TEXT));
    char info[200];
    if (game->multiplayer) {
        snprintf(info, sizeof(info), "P1: %d | P2: %d | Niveau: %d",
                 game->snake1.score, game->snake2.score, game->level);
    } else {
        snprintf(info, sizeof(info), "Score: %d | Niveau: %d | Longueur: %d | Vies: %d",
                 game->score, game->level, game->snake1.length,
                 (game->mode == MODE_ARCADE) ? game->snake1.lives : 0);
    }
    mvwprintw(game->win, 0, 2, "%s", info);
    
    // Power-ups actifs
    if (game->slow_timer > 0 || game->invincible_timer > 0 ||
        game->multiplier_timer > 0 || game->magnetic_timer > 0) {
        char powerups[100] = "Power-ups: ";
        if (game->slow_timer > 0) strcat(powerups, "SLOW ");
        if (game->invincible_timer > 0) strcat(powerups, "INV ");
        if (game->multiplier_timer > 0) strcat(powerups, "x2 ");
        if (game->magnetic_timer > 0) strcat(powerups, "MAG ");
        mvwprintw(game->win, game->grid_height + 1, 2, "%s", powerups);
    }
    
    if (game->paused) {
        const char *pause_msg = "PAUSE - Appuyez sur P pour continuer";
        int x = (game->grid_width - (int)strlen(pause_msg)) / 2;
        mvwprintw(game->win, game->grid_height / 2, x, "%s", pause_msg);
    }
    
    wattroff(game->win, COLOR_PAIR(COLOR_TEXT));
    wrefresh(game->win);
}

void load_top_scores(Game *game) {
    FILE *file = fopen(".snake_top_scores", "r");
    game->top_score_count = 0;
    
    if (file) {
        while (game->top_score_count < MAX_TOP_SCORES &&
               fscanf(file, "%d %19s %ld\n", &game->top_scores[game->top_score_count].score,
                      game->top_scores[game->top_score_count].name,
                      &game->top_scores[game->top_score_count].date) == 3) {
            game->top_score_count++;
        }
        fclose(file);
    }
}

void save_top_scores(Game *game) {
    FILE *file = fopen(".snake_top_scores", "w");
    if (file) {
        for (int i = 0; i < game->top_score_count; i++) {
            fprintf(file, "%d %s %ld\n", game->top_scores[i].score,
                    game->top_scores[i].name, game->top_scores[i].date);
        }
        fclose(file);
    }
}

void add_top_score(Game *game, int score) {
    if (game->top_score_count < MAX_TOP_SCORES ||
        score > game->top_scores[MAX_TOP_SCORES - 1].score) {
        
        int pos = game->top_score_count;
        for (int i = 0; i < game->top_score_count; i++) {
            if (score > game->top_scores[i].score) {
                pos = i;
                break;
            }
        }
        
        if (game->top_score_count < MAX_TOP_SCORES) {
            game->top_score_count++;
        }
        
        for (int i = game->top_score_count - 1; i > pos; i--) {
            game->top_scores[i] = game->top_scores[i - 1];
        }
        
        game->top_scores[pos].score = score;
        strncpy(game->top_scores[pos].name, "Player", 19);
        game->top_scores[pos].name[19] = '\0';
        game->top_scores[pos].date = time(NULL);
        
        save_top_scores(game);
    }
}

int show_main_menu() {
    WINDOW *menu_win = newwin(18, 55, (LINES - 18) / 2, (COLS - 55) / 2);
    keypad(menu_win, TRUE);
    
    int selected = 0;
    const char *options[] = {
        "Nouveau jeu (1 joueur)",
        "Nouveau jeu (2 joueurs)",
        "Meilleurs scores",
        "Quitter"
    };
    int num_options = 4;
    
    while (1) {
        werase(menu_win);
        box(menu_win, 0, 0);
        
        wattron(menu_win, COLOR_PAIR(COLOR_BORDER));
        box(menu_win, 0, 0);
        wattroff(menu_win, COLOR_PAIR(COLOR_BORDER));
        
        wattron(menu_win, COLOR_PAIR(COLOR_TEXT) | A_BOLD);
        mvwprintw(menu_win, 2, 18, "JEU DU SERPENT");
        wattroff(menu_win, COLOR_PAIR(COLOR_TEXT) | A_BOLD);
        
        for (int i = 0; i < num_options; i++) {
            if (i == selected) {
                wattron(menu_win, COLOR_PAIR(COLOR_SNAKE1_HEAD) | A_BOLD);
                mvwprintw(menu_win, 6 + i * 2, 15, "> %s", options[i]);
                wattroff(menu_win, COLOR_PAIR(COLOR_SNAKE1_HEAD) | A_BOLD);
            } else {
                wattron(menu_win, COLOR_PAIR(COLOR_TEXT));
                mvwprintw(menu_win, 6 + i * 2, 15, "  %s", options[i]);
                wattroff(menu_win, COLOR_PAIR(COLOR_TEXT));
            }
        }
        
        wattron(menu_win, COLOR_PAIR(COLOR_TEXT));
        mvwprintw(menu_win, 15, 12, "Utilisez les fleches pour naviguer");
        mvwprintw(menu_win, 16, 14, "Appuyez sur ENTER pour selectionner");
        
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
                return 3;
        }
    }
}

int show_game_mode_menu() {
    WINDOW *menu_win = newwin(14, 50, (LINES - 14) / 2, (COLS - 50) / 2);
    keypad(menu_win, TRUE);
    
    int selected = 0;
    const char *options[] = {
        "Classique",
        "Arcade (3 vies)",
        "Defi (obstacles)",
        "Libre (murs traversables)"
    };
    int num_options = 4;
    
    while (1) {
        werase(menu_win);
        box(menu_win, 0, 0);
        
        wattron(menu_win, COLOR_PAIR(COLOR_TEXT) | A_BOLD);
        mvwprintw(menu_win, 2, 15, "Choisir le mode");
        wattroff(menu_win, COLOR_PAIR(COLOR_TEXT) | A_BOLD);
        
        for (int i = 0; i < num_options; i++) {
            if (i == selected) {
                wattron(menu_win, COLOR_PAIR(COLOR_SNAKE1_HEAD) | A_BOLD);
                mvwprintw(menu_win, 5 + i, 15, "> %s", options[i]);
                wattroff(menu_win, COLOR_PAIR(COLOR_SNAKE1_HEAD) | A_BOLD);
            } else {
                wattron(menu_win, COLOR_PAIR(COLOR_TEXT));
                mvwprintw(menu_win, 5 + i, 15, "  %s", options[i]);
                wattroff(menu_win, COLOR_PAIR(COLOR_TEXT));
            }
        }
        
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
            case 27:  // ESC
                delwin(menu_win);
                return -1;
        }
    }
}

int show_difficulty_menu() {
    WINDOW *menu_win = newwin(12, 45, (LINES - 12) / 2, (COLS - 45) / 2);
    keypad(menu_win, TRUE);
    
    int selected = 1;  // Moyen par défaut
    const char *options[] = {
        "Facile",
        "Moyen",
        "Difficile",
        "Extreme"
    };
    int num_options = 4;
    
    while (1) {
        werase(menu_win);
        box(menu_win, 0, 0);
        
        wattron(menu_win, COLOR_PAIR(COLOR_TEXT) | A_BOLD);
        mvwprintw(menu_win, 2, 14, "Choisir difficulte");
        wattroff(menu_win, COLOR_PAIR(COLOR_TEXT) | A_BOLD);
        
        for (int i = 0; i < num_options; i++) {
            if (i == selected) {
                wattron(menu_win, COLOR_PAIR(COLOR_SNAKE1_HEAD) | A_BOLD);
                mvwprintw(menu_win, 5 + i, 15, "> %s", options[i]);
                wattroff(menu_win, COLOR_PAIR(COLOR_SNAKE1_HEAD) | A_BOLD);
            } else {
                wattron(menu_win, COLOR_PAIR(COLOR_TEXT));
                mvwprintw(menu_win, 5 + i, 15, "  %s", options[i]);
                wattroff(menu_win, COLOR_PAIR(COLOR_TEXT));
            }
        }
        
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
            case 27:
                delwin(menu_win);
                return 1;
        }
    }
}

void show_top_scores(Game *game) {
    WINDOW *score_win = newwin(18, 55, (LINES - 18) / 2, (COLS - 55) / 2);
    keypad(score_win, TRUE);
    
    load_top_scores(game);
    
    while (1) {
        werase(score_win);
        box(score_win, 0, 0);
        
        wattron(score_win, COLOR_PAIR(COLOR_TEXT) | A_BOLD);
        mvwprintw(score_win, 2, 18, "MEILLEURS SCORES");
        wattroff(score_win, COLOR_PAIR(COLOR_TEXT) | A_BOLD);
        
        wattron(score_win, COLOR_PAIR(COLOR_TEXT));
        if (game->top_score_count == 0) {
            mvwprintw(score_win, 8, 20, "Aucun score enregistre");
        } else {
            mvwprintw(score_win, 4, 10, "Rang  Score  Nom");
            for (int i = 0; i < game->top_score_count && i < 10; i++) {
                mvwprintw(score_win, 6 + i, 10, "%2d.   %5d  %s",
                          i + 1, game->top_scores[i].score, game->top_scores[i].name);
            }
        }
        
        mvwprintw(score_win, 16, 18, "Appuyez sur ESC pour revenir");
        wattroff(score_win, COLOR_PAIR(COLOR_TEXT));
        wrefresh(score_win);
        
        int ch = wgetch(score_win);
        if (ch == 27 || ch == 'q' || ch == 'Q') {  // ESC
            delwin(score_win);
            return;
        }
    }
}

int show_game_over(Game *game) {
    WINDOW *gameover_win = newwin(14, 50, (LINES - 14) / 2, (COLS - 50) / 2);
    keypad(gameover_win, TRUE);
    
    int selected = 0;
    const char *options[] = {
        "Rejouer",
        "Retour au menu",
        "Quitter"
    };
    int num_options = 3;
    
    // Calculer temps de jeu
    game->time_played = (int)(time(NULL) - game->start_time);
    
    // Ajouter au top scores
    add_top_score(game, game->score);
    
    while (1) {
        werase(gameover_win);
        box(gameover_win, 0, 0);
        
        wattron(gameover_win, COLOR_PAIR(COLOR_FOOD_NORMAL) | A_BOLD);
        if (game->multiplayer && game->winner > 0) {
            char msg[50];
            snprintf(msg, sizeof(msg), "JOUEUR %d GAGNE!", game->winner);
            mvwprintw(gameover_win, 2, (50 - strlen(msg)) / 2, "%s", msg);
        } else {
            mvwprintw(gameover_win, 2, 15, "GAME OVER!");
        }
        wattroff(gameover_win, COLOR_PAIR(COLOR_FOOD_NORMAL) | A_BOLD);
        
        wattron(gameover_win, COLOR_PAIR(COLOR_TEXT));
        char score_str[50];
        snprintf(score_str, sizeof(score_str), "Score final: %d", game->score);
        mvwprintw(gameover_win, 4, (50 - strlen(score_str)) / 2, "%s", score_str);
        
        char stats_str[80];
        snprintf(stats_str, sizeof(stats_str),
                 "Niveau: %d | Nourriture: %d | Temps: %d sec",
                 game->level, game->food_eaten, game->time_played);
        mvwprintw(gameover_win, 5, (50 - strlen(stats_str)) / 2, "%s", stats_str);
        
        for (int i = 0; i < num_options; i++) {
            if (i == selected) {
                wattron(gameover_win, COLOR_PAIR(COLOR_SNAKE1_HEAD) | A_BOLD);
                mvwprintw(gameover_win, 8 + i, (50 - strlen(options[i]) - 4) / 2,
                          "> %s", options[i]);
                wattroff(gameover_win, COLOR_PAIR(COLOR_SNAKE1_HEAD) | A_BOLD);
            } else {
                mvwprintw(gameover_win, 8 + i, (50 - strlen(options[i]) - 2) / 2,
                          "  %s", options[i]);
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

void game_loop(Game *game) {
    clock_t last_move = clock();
    
    while (!game->game_over) {
        if (game->multiplayer) {
            handle_input_multiplayer(game);
        } else {
            handle_input(game);
        }
        
        clock_t current = clock();
        double elapsed = ((double)(current - last_move) / CLOCKS_PER_SEC) * 1000;
        
        if (elapsed >= game->speed && !game->paused) {
            move_snake(game, &game->snake1);
            if (game->multiplayer) {
                move_snake(game, &game->snake2);
            }
            update_powerups(game);
            last_move = current;
        }
        
        draw_game(game);
        usleep(10000);
    }
}

int main() {
    initscr();
    noecho();
    curs_set(0);
    init_colors();
    init_theme_colors(THEME_CLASSIC);
    
    srand(time(NULL));
    
    int running = 1;
    
    while (running) {
        int menu_choice = show_main_menu();
        
        if (menu_choice == 0 || menu_choice == 1) {  // Nouveau jeu
            int multiplayer = (menu_choice == 1);
            
            int mode_choice = show_game_mode_menu();
            if (mode_choice < 0) continue;
            
            int diff_choice = show_difficulty_menu();
            if (diff_choice < 0) continue;
            
            Game game;
            init_game(&game, mode_choice, diff_choice, multiplayer);
            
            game_loop(&game);
            
            int gameover_choice = show_game_over(&game);
            
            if (gameover_choice == 0) {  // Rejouer
                continue;
            } else if (gameover_choice == 1) {  // Retour au menu
                continue;
            } else {  // Quitter
                running = 0;
            }
        } else if (menu_choice == 2) {  // Meilleurs scores
            Game game;
            load_top_scores(&game);
            show_top_scores(&game);
        } else {  // Quitter
            running = 0;
        }
    }
    
    endwin();
    return 0;
}
