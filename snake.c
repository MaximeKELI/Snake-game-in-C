#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <SDL2/SDL.h>

// ===== CONSTANTES =====
#define CELL_SIZE 20
#define GRID_WIDTH 60
#define GRID_HEIGHT 20
#define SCREEN_WIDTH (GRID_WIDTH * CELL_SIZE)
#define SCREEN_HEIGHT (GRID_HEIGHT * CELL_SIZE + 100)  // +100 pour le HUD
#define MAX_LENGTH 1000
#define MAX_OBSTACLES 50
#define MAX_FOOD 5
#define MAX_TOP_SCORES 10
#define POWERUP_DURATION 100

// ===== ENUMS =====
typedef enum {
    FOOD_NORMAL = 0, FOOD_GOLDEN, FOOD_POISON, FOOD_FAST, FOOD_BONUS
} FoodType;

typedef enum {
    POWERUP_NONE = 0, POWERUP_SLOW, POWERUP_INVINCIBLE, POWERUP_MULTIPLIER, POWERUP_MAGNETIC
} PowerUpType;

typedef enum {
    MODE_CLASSIC = 0, MODE_ARCADE, MODE_CHALLENGE, MODE_FREE
} GameMode;

typedef enum {
    DIFF_EASY = 0, DIFF_MEDIUM, DIFF_HARD, DIFF_EXTREME
} Difficulty;

typedef enum { UP = 0, RIGHT, DOWN, LEFT } Direction;

// ===== STRUCTURES =====
typedef struct {
    int x;
    int y;
} Position;

typedef struct {
    Position pos;
    FoodType type;
    int timer;
    int pulse;
} Food;

typedef struct {
    Position pos;
    PowerUpType type;
    int timer;
    int active;
} PowerUp;

typedef struct {
    Position pos;
    int timer;
    int type;
    Position portal_dest;
} Obstacle;

typedef struct {
    Position body[MAX_LENGTH];
    int length;
    Direction direction;
    int lives;
    int score;
    int multiplier;
    int combo_count;
    Uint32 last_food_time;
    SDL_Color color_head;
    SDL_Color color_body;
} Snake;

typedef struct {
    int score;
    int level;
    char name[20];
    time_t date;
} TopScore;

typedef struct {
    Snake snake1;
    Snake snake2;
    int multiplayer;
    Food foods[MAX_FOOD];
    int food_count;
    PowerUp powerup;
    Obstacle obstacles[MAX_OBSTACLES];
    int obstacle_count;
    int score;
    int level;
    int speed;
    int base_speed;
    int game_over;
    int paused;
    int winner;
    GameMode mode;
    Difficulty difficulty;
    int grid_width;
    int grid_height;
    int food_eaten;
    int time_played;
    Uint32 start_time;
    int slow_timer;
    int invincible_timer;
    int multiplier_timer;
    int magnetic_timer;
    TopScore top_scores[MAX_TOP_SCORES];
    int top_score_count;
} Game;

// SDL globals
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
int screen_w = SCREEN_WIDTH;
int screen_h = SCREEN_HEIGHT;

// ===== PROTOTYPES =====
int init_sdl();
void cleanup_sdl();
void init_snake(Snake *snake, int start_x, int start_y, int player_num);
void init_game(Game *game, GameMode mode, Difficulty diff, int multiplayer);
Position generate_random_position(Game *game);
int is_position_valid(Game *game, Position pos, int check_snake);
void generate_food(Game *game);
void generate_powerup(Game *game);
void generate_obstacles(Game *game);
void move_snake(Game *game, Snake *snake);
void update_powerups(Game *game);
void check_food_collision(Game *game, Snake *snake);
void check_obstacle_collision(Game *game, Snake *snake);
void draw_rect(int x, int y, int w, int h, SDL_Color color);
void draw_game(Game *game);
void handle_input(Game *game, SDL_Event *e);
void load_top_scores(Game *game);
void save_top_scores(Game *game);
void add_top_score(Game *game, int score);
int show_main_menu();
int show_game_mode_menu();
int show_difficulty_menu();
int show_game_over_menu(Game *game);
void game_loop(Game *game);

// ===== IMPLÉMENTATION =====

int init_sdl() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Erreur SDL: %s\n", SDL_GetError());
        return 0;
    }
    
    window = SDL_CreateWindow("Jeu du Serpent",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              screen_w, screen_h,
                              SDL_WINDOW_SHOWN);
    
    if (!window) {
        fprintf(stderr, "Erreur création fenêtre: %s\n", SDL_GetError());
        SDL_Quit();
        return 0;
    }
    
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        fprintf(stderr, "Erreur création renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }
    
    return 1;
}

void cleanup_sdl() {
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
}

void draw_rect(int x, int y, int w, int h, SDL_Color color) {
    SDL_Rect rect = {x, y, w, h};
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);
}

void init_snake(Snake *snake, int start_x, int start_y, int player_num) {
    snake->length = 3;
    snake->direction = RIGHT;
    snake->lives = 3;
    snake->score = 0;
    snake->multiplier = 1;
    snake->combo_count = 0;
    snake->last_food_time = SDL_GetTicks();
    
    if (player_num == 1) {
        snake->color_head = (SDL_Color){0, 255, 0, 255};  // Vert
        snake->color_body = (SDL_Color){255, 255, 0, 255};  // Jaune
    } else {
        snake->color_head = (SDL_Color){0, 255, 255, 255};  // Cyan
        snake->color_body = (SDL_Color){0, 0, 255, 255};  // Bleu
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
    
    switch (diff) {
        case DIFF_EASY:
            game->grid_width = 80;
            game->grid_height = 30;
            game->base_speed = 200;
            break;
        case DIFF_MEDIUM:
            game->grid_width = GRID_WIDTH;
            game->grid_height = GRID_HEIGHT;
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
    
    int start_x = game->grid_width / 2;
    int start_y = game->grid_height / 2;
    init_snake(&game->snake1, start_x, start_y, 1);
    
    if (multiplayer) {
        init_snake(&game->snake2, start_x - 10, start_y, 2);
    }
    
    game->score = 0;
    game->level = 1;
    game->speed = game->base_speed;
    game->game_over = 0;
    game->paused = 0;
    game->winner = 0;
    game->food_count = 1;
    game->obstacle_count = 0;
    game->food_eaten = 0;
    game->start_time = SDL_GetTicks();
    
    game->powerup.active = 0;
    game->slow_timer = 0;
    game->invincible_timer = 0;
    game->multiplier_timer = 0;
    game->magnetic_timer = 0;
    
    generate_food(game);
    if (mode == MODE_CHALLENGE) {
        generate_obstacles(game);
    }
    
    load_top_scores(game);
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
    
    for (int i = 0; i < game->obstacle_count; i++) {
        if (game->obstacles[i].pos.x == pos.x && game->obstacles[i].pos.y == pos.y)
            return 0;
    }
    
    for (int i = 0; i < game->food_count; i++) {
        if (game->foods[i].pos.x == pos.x && game->foods[i].pos.y == pos.y)
            return 0;
    }
    
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
            int r = rand() % 100;
            if (r < 50) game->foods[i].type = FOOD_NORMAL;
            else if (r < 70) game->foods[i].type = FOOD_GOLDEN;
            else if (r < 85) game->foods[i].type = FOOD_POISON;
            else if (r < 95) game->foods[i].type = FOOD_FAST;
            else game->foods[i].type = FOOD_BONUS;
            game->foods[i].timer = 0;
            game->foods[i].pulse = 0;
        }
    }
}

void generate_powerup(Game *game) {
    if (game->powerup.active) return;
    if (rand() % 100 < 15) {
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
            game->obstacles[game->obstacle_count].type = (rand() % 10 < 2) ? 2 : 0;
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
    
    if (game->mode == MODE_FREE) {
        if (head.x < 0) head.x = game->grid_width - 1;
        if (head.x >= game->grid_width) head.x = 0;
        if (head.y < 0) head.y = game->grid_height - 1;
        if (head.y >= game->grid_height) head.y = 0;
    } else {
        if (head.x < 0 || head.x >= game->grid_width ||
            head.y < 0 || head.y >= game->grid_height) {
            if (game->mode == MODE_ARCADE && snake->lives > 0) {
                snake->lives--;
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
    
    if (game->invincible_timer == 0) {
        for (int i = 1; i < snake->length; i++) {
            if (head.x == snake->body[i].x && head.y == snake->body[i].y) {
                if (game->mode == MODE_ARCADE && snake->lives > 0) {
                    snake->lives--;
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
    
    if (game->multiplayer) {
        Snake *other = (snake == &game->snake1) ? &game->snake2 : &game->snake1;
        for (int i = 0; i < other->length; i++) {
            if (head.x == other->body[i].x && head.y == other->body[i].y) {
                if (game->invincible_timer == 0) {
                    if (game->mode == MODE_ARCADE && snake->lives > 0) {
                        snake->lives--;
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
    
    for (int i = snake->length; i > 0; i--) {
        snake->body[i] = snake->body[i - 1];
    }
    snake->body[0] = head;
    
    check_obstacle_collision(game, snake);
    check_food_collision(game, snake);
    
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
                default: break;
            }
            game->powerup.active = 0;
        }
    }
}

void check_obstacle_collision(Game *game, Snake *snake) {
    Position head = snake->body[0];
    
    for (int i = 0; i < game->obstacle_count; i++) {
        if (head.x == game->obstacles[i].pos.x && head.y == game->obstacles[i].pos.y) {
            if (game->obstacles[i].type == 2) {
                snake->body[0] = game->obstacles[i].portal_dest;
            } else {
                if (game->invincible_timer == 0) {
                    if (game->mode == MODE_ARCADE && snake->lives > 0) {
                        snake->lives--;
                        int start_x = game->grid_width / 2;
                        int start_y = game->grid_height / 2;
                        snake->length = 3;
                        for (int j = 0; j < snake->length; j++) {
                            snake->body[j].x = start_x - j;
                            snake->body[j].y = start_y;
                        }
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
                case FOOD_NORMAL: points = 10; break;
                case FOOD_GOLDEN: points = 50; break;
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
                case FOOD_BONUS: points = 100; break;
            }
            
            Uint32 now = SDL_GetTicks();
            if (now - snake->last_food_time < 2000) {
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
            
            int new_level = (game->score / 100) + 1;
            if (new_level > game->level) {
                game->level = new_level;
                if (game->slow_timer == 0) {
                    game->speed = game->base_speed - (game->level - 1) * 5;
                    if (game->speed < 30) game->speed = 30;
                }
            }
            
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
    
    if (game->invincible_timer > 0) game->invincible_timer--;
    
    if (game->multiplier_timer > 0) {
        game->multiplier_timer--;
        if (game->multiplier_timer == 0) {
            game->snake1.multiplier = 1;
            if (game->multiplayer) game->snake2.multiplier = 1;
        }
    }
    
    if (game->magnetic_timer > 0) game->magnetic_timer--;
    
    for (int i = 0; i < game->food_count; i++) {
        game->foods[i].timer++;
        game->foods[i].pulse = (game->foods[i].pulse + 1) % 10;
    }
}

void draw_game(Game *game) {
    // Fond noir
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    // Bordure
    SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
    SDL_Rect border = {0, 0, game->grid_width * CELL_SIZE, game->grid_height * CELL_SIZE};
    SDL_RenderDrawRect(renderer, &border);
    
    // Obstacles
    for (int i = 0; i < game->obstacle_count; i++) {
        int x = game->obstacles[i].pos.x * CELL_SIZE;
        int y = game->obstacles[i].pos.y * CELL_SIZE;
        SDL_Color color = (game->obstacles[i].type == 2) ? 
            (SDL_Color){255, 0, 255, 255} : (SDL_Color){255, 255, 255, 255};
        draw_rect(x, y, CELL_SIZE, CELL_SIZE, color);
    }
    
    // Nourriture
    for (int i = 0; i < game->food_count; i++) {
        int x = game->foods[i].pos.x * CELL_SIZE;
        int y = game->foods[i].pos.y * CELL_SIZE;
        SDL_Color color;
        switch (game->foods[i].type) {
            case FOOD_NORMAL: color = (SDL_Color){255, 0, 0, 255}; break;
            case FOOD_GOLDEN: color = (SDL_Color){255, 215, 0, 255}; break;
            case FOOD_POISON: color = (SDL_Color){255, 0, 255, 255}; break;
            case FOOD_FAST: color = (SDL_Color){0, 255, 255, 255}; break;
            case FOOD_BONUS: color = (SDL_Color){255, 255, 255, 255}; break;
            default: color = (SDL_Color){255, 0, 0, 255}; break;
        }
        draw_rect(x, y, CELL_SIZE, CELL_SIZE, color);
    }
    
    // Power-up
    if (game->powerup.active) {
        int x = game->powerup.pos.x * CELL_SIZE;
        int y = game->powerup.pos.y * CELL_SIZE;
        draw_rect(x, y, CELL_SIZE, CELL_SIZE, (SDL_Color){0, 255, 0, 255});
    }
    
    // Serpent 1
    for (int i = 0; i < game->snake1.length; i++) {
        int x = game->snake1.body[i].x * CELL_SIZE;
        int y = game->snake1.body[i].y * CELL_SIZE;
        SDL_Color color = (i == 0) ? game->snake1.color_head : game->snake1.color_body;
        if (i == 0 && game->invincible_timer > 0) {
            // Clignotement pour invincibilité
            if ((SDL_GetTicks() / 100) % 2) {
                draw_rect(x, y, CELL_SIZE, CELL_SIZE, color);
            }
        } else {
            draw_rect(x, y, CELL_SIZE, CELL_SIZE, color);
        }
    }
    
    // Serpent 2 (multijoueur)
    if (game->multiplayer) {
        for (int i = 0; i < game->snake2.length; i++) {
            int x = game->snake2.body[i].x * CELL_SIZE;
            int y = game->snake2.body[i].y * CELL_SIZE;
            SDL_Color color = (i == 0) ? game->snake2.color_head : game->snake2.color_body;
            draw_rect(x, y, CELL_SIZE, CELL_SIZE, color);
        }
    }
    
    // HUD (informations)
    char info[200];
    if (game->multiplayer) {
        snprintf(info, sizeof(info), "P1: %d | P2: %d | Niveau: %d",
                 game->snake1.score, game->snake2.score, game->level);
    } else {
        snprintf(info, sizeof(info), "Score: %d | Niveau: %d | Longueur: %d | Vies: %d",
                 game->score, game->level, game->snake1.length,
                 (game->mode == MODE_ARCADE) ? game->snake1.lives : 0);
    }
    
    // Pour le texte, on utilisera une approche simple (pas de TTF pour l'instant)
    // On affichera juste les informations dans le terminal ou on les dessinera avec des rectangles
    
    if (game->paused) {
        // Dessiner un rectangle semi-transparent pour la pause
        SDL_SetRenderDrawColor(renderer, 128, 128, 128, 128);
        SDL_Rect pause_rect = {screen_w / 2 - 100, screen_h / 2 - 20, 200, 40};
        SDL_RenderFillRect(renderer, &pause_rect);
    }
    
    SDL_RenderPresent(renderer);
}

void handle_input(Game *game, SDL_Event *e) {
    if (e->type == SDL_KEYDOWN) {
        switch (e->key.keysym.sym) {
            case SDLK_UP:
            case SDLK_w:
                if (game->snake1.direction != DOWN)
                    game->snake1.direction = UP;
                break;
            case SDLK_RIGHT:
            case SDLK_d:
                if (game->snake1.direction != LEFT)
                    game->snake1.direction = RIGHT;
                break;
            case SDLK_DOWN:
            case SDLK_s:
                if (game->snake1.direction != UP)
                    game->snake1.direction = DOWN;
                break;
            case SDLK_LEFT:
            case SDLK_a:
                if (game->snake1.direction != RIGHT)
                    game->snake1.direction = LEFT;
                break;
            case SDLK_p:
                if (!game->game_over)
                    game->paused = !game->paused;
                break;
            case SDLK_q:
            case SDLK_ESCAPE:
                game->game_over = 1;
                break;
        }
    }
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
        (game->top_score_count > 0 && score > game->top_scores[game->top_score_count - 1].score)) {
        int pos = game->top_score_count;
        for (int i = 0; i < game->top_score_count; i++) {
            if (score > game->top_scores[i].score) {
                pos = i;
                break;
            }
        }
        if (game->top_score_count < MAX_TOP_SCORES) game->top_score_count++;
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

// Menus simplifiés - pour l'instant on utilise des valeurs par défaut
// Dans une version complète, on pourrait créer des menus graphiques avec SDL
int show_main_menu() {
    // Menu simple - on retourne directement 0 pour commencer le jeu
    // Dans une vraie implémentation, on créerait des boutons graphiques
    return 0;
}

int show_game_mode_menu() {
    return MODE_CLASSIC;
}

int show_difficulty_menu() {
    return DIFF_MEDIUM;
}

int show_game_over_menu(Game *game) {
    game->time_played = (SDL_GetTicks() - game->start_time) / 1000;
    add_top_score(game, game->score);
    
    // Attendre un peu puis quitter
    SDL_Delay(2000);
    return 1;  // Retour au menu
}

void game_loop(Game *game) {
    Uint32 last_move = SDL_GetTicks();
    SDL_Event e;
    
    while (!game->game_over) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                game->game_over = 1;
                break;
            }
            handle_input(game, &e);
        }
        
        Uint32 current = SDL_GetTicks();
        if (current - last_move >= (Uint32)game->speed && !game->paused) {
            move_snake(game, &game->snake1);
            if (game->multiplayer) {
                move_snake(game, &game->snake2);
            }
            update_powerups(game);
            last_move = current;
        }
        
        draw_game(game);
        SDL_Delay(10);
    }
}

int main(int argc, char *argv[]) {
    if (!init_sdl()) {
        return 1;
    }
    
    srand(time(NULL));
    
    Game game;
    init_game(&game, MODE_CLASSIC, DIFF_MEDIUM, 0);
    
    game_loop(&game);
    
    int choice = show_game_over_menu(&game);
    
    cleanup_sdl();
    return 0;
}
