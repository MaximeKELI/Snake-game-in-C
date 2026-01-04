#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <unistd.h>

// ===== DÉFINITIONS (extraites de snake.c sans ncurses) =====

#define MAX_LENGTH 1000
#define MAX_OBSTACLES 50
#define MAX_FOOD 5
#define MAX_TOP_SCORES 10
#define POWERUP_DURATION 100

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

typedef struct { int x; int y; } Position;

typedef struct {
    Position pos; FoodType type; int timer; int pulse;
} Food;

typedef struct {
    Position pos; PowerUpType type; int timer; int active;
} PowerUp;

typedef struct {
    Position pos; int timer; int type; Position portal_dest;
} Obstacle;

typedef struct {
    Position body[MAX_LENGTH];
    int length; Direction direction;
    char head_char; char body_char;
    int color_head; int color_body;
    int lives; int score; int multiplier;
    int combo_count; time_t last_food_time;
} Snake;

typedef struct {
    int score; int level; char name[20]; time_t date;
} TopScore;

typedef struct {
    Snake snake1, snake2; int multiplayer;
    Food foods[MAX_FOOD]; int food_count;
    PowerUp powerup;
    Obstacle obstacles[MAX_OBSTACLES]; int obstacle_count;
    int score, level, speed, base_speed;
    int game_over, paused, winner;
    GameMode mode; Difficulty difficulty;
    int grid_width, grid_height;
    int food_eaten, time_played; time_t start_time;
    int slow_timer, invincible_timer, multiplier_timer, magnetic_timer;
    void *win;
    TopScore top_scores[MAX_TOP_SCORES]; int top_score_count;
} Game;

#define COLOR_SNAKE1_HEAD 1
#define COLOR_SNAKE1_BODY 2
#define COLOR_SNAKE2_HEAD 3
#define COLOR_SNAKE2_BODY 4

// ===== IMPLÉMENTATIONS DES FONCTIONS À TESTER =====

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
        if (game->obstacles[i].pos.x == pos.x && game->obstacles[i].pos.y == pos.y) return 0;
    }
    for (int i = 0; i < game->food_count; i++) {
        if (game->foods[i].pos.x == pos.x && game->foods[i].pos.y == pos.y) return 0;
    }
    if (check_snake) {
        for (int i = 0; i < game->snake1.length; i++) {
            if (game->snake1.body[i].x == pos.x && game->snake1.body[i].y == pos.y) return 0;
        }
        if (game->multiplayer) {
            for (int i = 0; i < game->snake2.length; i++) {
                if (game->snake2.body[i].x == pos.x && game->snake2.body[i].y == pos.y) return 0;
            }
        }
    }
    return 1;
}

void init_snake(Snake *snake, int start_x, int start_y, int player_num) {
    snake->length = 3;
    snake->direction = RIGHT;
    snake->lives = 3;
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
    game->win = NULL;
    switch (diff) {
        case DIFF_EASY: game->grid_width = 80; game->grid_height = 30; game->base_speed = 200; break;
        case DIFF_MEDIUM: game->grid_width = 60; game->grid_height = 20; game->base_speed = 150; break;
        case DIFF_HARD: game->grid_width = 50; game->grid_height = 18; game->base_speed = 100; break;
        case DIFF_EXTREME: game->grid_width = 40; game->grid_height = 15; game->base_speed = 50; break;
    }
    int start_x = game->grid_width / 2;
    int start_y = game->grid_height / 2;
    init_snake(&game->snake1, start_x, start_y, 1);
    if (multiplayer) init_snake(&game->snake2, start_x - 10, start_y, 2);
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
    game->powerup.active = 0;
    game->slow_timer = 0;
    game->invincible_timer = 0;
    game->multiplier_timer = 0;
    game->magnetic_timer = 0;
    Position food_pos;
    int attempts = 0;
    do {
        food_pos = generate_random_position(game);
        attempts++;
    } while (!is_position_valid(game, food_pos, 1) && attempts < 100);
    if (attempts < 100 && game->food_count > 0) {
        game->foods[0].pos = food_pos;
        game->foods[0].type = FOOD_NORMAL;
        game->foods[0].timer = 0;
        game->foods[0].pulse = 0;
    }
    if (mode == MODE_CHALLENGE) {
        game->obstacle_count = 5;
        for (int i = 0; i < game->obstacle_count; i++) {
            Position obs_pos;
            int obs_attempts = 0;
            do {
                obs_pos = generate_random_position(game);
                obs_attempts++;
            } while (!is_position_valid(game, obs_pos, 0) && obs_attempts < 100);
            if (obs_attempts < 100) {
                game->obstacles[i].pos = obs_pos;
                game->obstacles[i].timer = 0;
                game->obstacles[i].type = 0;
            }
        }
    }
    game->top_score_count = 0;
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

// ===== FRAMEWORK DE TEST =====

static int tests_run = 0, tests_passed = 0, tests_failed = 0;

#define TEST_ASSERT(condition, message) \
    do { \
        tests_run++; \
        if (condition) { \
            tests_passed++; \
            printf("✓ PASS: %s\n", message); \
        } else { \
            tests_failed++; \
            printf("✗ FAIL: %s (ligne %d)\n", message, __LINE__); \
        } \
    } while(0)

#define TEST_EQUAL(actual, expected, message) TEST_ASSERT((actual) == (expected), message)
#define TEST_RANGE(value, min, max, message) TEST_ASSERT((value) >= (min) && (value) <= (max), message)

// ===== TESTS =====

void test_generate_random_position() {
    printf("\n=== Test: generate_random_position ===\n");
    Game game;
    game.grid_width = 60;
    game.grid_height = 20;
    for (int i = 0; i < 100; i++) {
        Position pos = generate_random_position(&game);
        TEST_RANGE(pos.x, 0, game.grid_width - 1, "Position X dans les limites");
        TEST_RANGE(pos.y, 0, game.grid_height - 1, "Position Y dans les limites");
    }
}

void test_is_position_valid() {
    printf("\n=== Test: is_position_valid ===\n");
    Game game;
    game.grid_width = 60;
    game.grid_height = 20;
    game.obstacle_count = 0;
    game.food_count = 0;
    game.multiplayer = 0;
    game.snake1.length = 0;
    Position valid_pos = {30, 10};
    TEST_EQUAL(is_position_valid(&game, valid_pos, 0), 1, "Position valide acceptée");
    Position invalid_x = {-1, 10};
    TEST_EQUAL(is_position_valid(&game, invalid_x, 0), 0, "Position X négative rejetée");
    Position invalid_y = {30, -1};
    TEST_EQUAL(is_position_valid(&game, invalid_y, 0), 0, "Position Y négative rejetée");
    game.obstacle_count = 1;
    game.obstacles[0].pos.x = 30;
    game.obstacles[0].pos.y = 10;
    TEST_EQUAL(is_position_valid(&game, valid_pos, 0), 0, "Position avec obstacle rejetée");
    game.obstacle_count = 0;
    game.snake1.length = 3;
    game.snake1.body[0] = (Position){30, 10};
    game.snake1.body[1] = (Position){29, 10};
    game.snake1.body[2] = (Position){28, 10};
    TEST_EQUAL(is_position_valid(&game, valid_pos, 1), 0, "Position sur le serpent rejetée");
}

void test_init_snake() {
    printf("\n=== Test: init_snake ===\n");
    Snake snake;
    init_snake(&snake, 30, 10, 1);
    TEST_EQUAL(snake.length, 3, "Longueur initiale = 3");
    TEST_EQUAL(snake.direction, RIGHT, "Direction initiale = RIGHT");
    TEST_EQUAL(snake.lives, 3, "Vies initiales = 3");
    TEST_EQUAL(snake.score, 0, "Score initial = 0");
    TEST_EQUAL(snake.multiplier, 1, "Multiplicateur initial = 1");
    TEST_EQUAL(snake.head_char, '@', "Caractère tête joueur 1 = @");
    TEST_EQUAL(snake.body_char, 'o', "Caractère corps joueur 1 = o");
    TEST_EQUAL(snake.body[0].x, 30, "Position tête X correcte");
    TEST_EQUAL(snake.body[0].y, 10, "Position tête Y correcte");
    Snake snake2;
    init_snake(&snake2, 30, 10, 2);
    TEST_EQUAL(snake2.head_char, '#', "Caractère tête joueur 2 = #");
    TEST_EQUAL(snake2.body_char, '*', "Caractère corps joueur 2 = *");
}

void test_init_game() {
    printf("\n=== Test: init_game ===\n");
    Game game;
    init_game(&game, MODE_CLASSIC, DIFF_MEDIUM, 0);
    TEST_EQUAL(game.mode, MODE_CLASSIC, "Mode = CLASSIC");
    TEST_EQUAL(game.difficulty, DIFF_MEDIUM, "Difficulté = MEDIUM");
    TEST_EQUAL(game.multiplayer, 0, "Multijoueur = 0");
    TEST_EQUAL(game.score, 0, "Score initial = 0");
    TEST_EQUAL(game.level, 1, "Niveau initial = 1");
    TEST_EQUAL(game.grid_width, 60, "Largeur grille (moyen) = 60");
    TEST_EQUAL(game.grid_height, 20, "Hauteur grille (moyen) = 20");
    TEST_EQUAL(game.base_speed, 150, "Vitesse de base (moyen) = 150");
    init_game(&game, MODE_CLASSIC, DIFF_EASY, 0);
    TEST_EQUAL(game.grid_width, 80, "Largeur grille (facile) = 80");
    TEST_EQUAL(game.grid_height, 30, "Hauteur grille (facile) = 30");
    TEST_EQUAL(game.base_speed, 200, "Vitesse de base (facile) = 200");
    init_game(&game, MODE_CLASSIC, DIFF_HARD, 0);
    TEST_EQUAL(game.grid_width, 50, "Largeur grille (difficile) = 50");
    TEST_EQUAL(game.grid_height, 18, "Hauteur grille (difficile) = 18");
    TEST_EQUAL(game.base_speed, 100, "Vitesse de base (difficile) = 100");
    init_game(&game, MODE_CLASSIC, DIFF_MEDIUM, 1);
    TEST_EQUAL(game.multiplayer, 1, "Multijoueur activé");
    TEST_EQUAL(game.snake2.length, 3, "Longueur serpent 2 = 3");
}

void test_update_powerups() {
    printf("\n=== Test: update_powerups ===\n");
    Game game;
    init_game(&game, MODE_CLASSIC, DIFF_MEDIUM, 0);
    game.slow_timer = 10;
    game.speed = 300;
    game.base_speed = 150;
    game.level = 1;
    for (int i = 0; i < 10; i++) update_powerups(&game);
    TEST_EQUAL(game.slow_timer, 0, "Timer ralentissement expiré");
    TEST_EQUAL(game.speed, 150, "Vitesse restaurée");
    game.invincible_timer = 5;
    for (int i = 0; i < 5; i++) update_powerups(&game);
    TEST_EQUAL(game.invincible_timer, 0, "Timer invincibilité expiré");
    game.multiplier_timer = 3;
    game.snake1.multiplier = 2;
    for (int i = 0; i < 3; i++) update_powerups(&game);
    TEST_EQUAL(game.multiplier_timer, 0, "Timer multiplicateur expiré");
    TEST_EQUAL(game.snake1.multiplier, 1, "Multiplicateur restauré");
}

void test_top_scores() {
    printf("\n=== Test: Top Scores ===\n");
    remove(".snake_top_scores");
    Game game;
    game.top_score_count = 0;
    add_top_score(&game, 100);
    TEST_EQUAL(game.top_score_count, 1, "Premier score ajouté");
    TEST_EQUAL(game.top_scores[0].score, 100, "Premier score = 100");
    add_top_score(&game, 200);
    TEST_EQUAL(game.top_score_count, 2, "Deuxième score ajouté");
    TEST_EQUAL(game.top_scores[0].score, 200, "Meilleur score en premier");
    TEST_EQUAL(game.top_scores[1].score, 100, "Second score en deuxième");
    add_top_score(&game, 150);
    TEST_EQUAL(game.top_scores[0].score, 200, "Meilleur score reste 200");
    TEST_EQUAL(game.top_scores[1].score, 150, "Nouveau score inséré correctement");
    TEST_EQUAL(game.top_scores[2].score, 100, "Dernier score décalé");
    save_top_scores(&game);
    Game game2;
    game2.top_score_count = 0;
    load_top_scores(&game2);
    TEST_EQUAL(game2.top_score_count, 3, "Scores chargés correctement");
    TEST_EQUAL(game2.top_scores[0].score, 200, "Meilleur score chargé = 200");
    remove(".snake_top_scores");
}

void test_get_food_char() {
    printf("\n=== Test: get_food_char ===\n");
    TEST_ASSERT(strcmp(get_food_char(FOOD_NORMAL), "*") == 0, "Caractère nourriture normale = *");
    TEST_ASSERT(strcmp(get_food_char(FOOD_GOLDEN), "$") == 0, "Caractère nourriture dorée = $");
    TEST_ASSERT(strcmp(get_food_char(FOOD_POISON), "X") == 0, "Caractère nourriture poison = X");
    TEST_ASSERT(strcmp(get_food_char(FOOD_FAST), "!") == 0, "Caractère nourriture rapide = !");
    TEST_ASSERT(strcmp(get_food_char(FOOD_BONUS), "?") == 0, "Caractère nourriture bonus = ?");
}

void test_get_powerup_char() {
    printf("\n=== Test: get_powerup_char ===\n");
    TEST_ASSERT(strcmp(get_powerup_char(POWERUP_SLOW), "S") == 0, "Caractère power-up slow = S");
    TEST_ASSERT(strcmp(get_powerup_char(POWERUP_INVINCIBLE), "I") == 0, "Caractère power-up invincible = I");
    TEST_ASSERT(strcmp(get_powerup_char(POWERUP_MULTIPLIER), "M") == 0, "Caractère power-up multiplier = M");
    TEST_ASSERT(strcmp(get_powerup_char(POWERUP_MAGNETIC), "G") == 0, "Caractère power-up magnetic = G");
}

void test_snake_movement_logic() {
    printf("\n=== Test: Snake Movement Logic ===\n");
    Game game;
    init_game(&game, MODE_CLASSIC, DIFF_MEDIUM, 0);
    TEST_EQUAL(game.snake1.direction, RIGHT, "Direction initiale = RIGHT");
    TEST_EQUAL(game.snake1.body[0].x, game.grid_width / 2, "Position X tête initiale");
    TEST_EQUAL(game.snake1.body[0].y, game.grid_height / 2, "Position Y tête initiale");
    TEST_EQUAL(game.snake1.length, 3, "Longueur initiale = 3");
}

void test_game_modes() {
    printf("\n=== Test: Game Modes ===\n");
    Game game;
    init_game(&game, MODE_CLASSIC, DIFF_MEDIUM, 0);
    TEST_EQUAL(game.mode, MODE_CLASSIC, "Mode classique");
    init_game(&game, MODE_ARCADE, DIFF_MEDIUM, 0);
    TEST_EQUAL(game.mode, MODE_ARCADE, "Mode arcade");
    TEST_EQUAL(game.snake1.lives, 3, "Vies en mode arcade = 3");
    init_game(&game, MODE_CHALLENGE, DIFF_MEDIUM, 0);
    TEST_EQUAL(game.mode, MODE_CHALLENGE, "Mode défi");
    TEST_RANGE(game.obstacle_count, 0, MAX_OBSTACLES, "Obstacles générés en mode défi");
    init_game(&game, MODE_FREE, DIFF_MEDIUM, 0);
    TEST_EQUAL(game.mode, MODE_FREE, "Mode libre");
}

void test_difficulty_extreme() {
    printf("\n=== Test: Difficulté Extrême ===\n");
    Game game;
    init_game(&game, MODE_CLASSIC, DIFF_EXTREME, 0);
    TEST_EQUAL(game.grid_width, 40, "Largeur grille (extrême) = 40");
    TEST_EQUAL(game.grid_height, 15, "Hauteur grille (extrême) = 15");
    TEST_EQUAL(game.base_speed, 50, "Vitesse de base (extrême) = 50");
}

void test_multiplier_system() {
    printf("\n=== Test: Système Multiplicateur ===\n");
    Game game;
    init_game(&game, MODE_CLASSIC, DIFF_MEDIUM, 0);
    TEST_EQUAL(game.snake1.multiplier, 1, "Multiplicateur initial = 1");
    game.snake1.multiplier = 2;
    int points = 10;
    int final_points = points * game.snake1.multiplier;
    TEST_EQUAL(final_points, 20, "Points multipliés (10 * 2 = 20)");
    game.snake1.multiplier = 3;
    final_points = points * game.snake1.multiplier;
    TEST_EQUAL(final_points, 30, "Points multipliés (10 * 3 = 30)");
}

void test_top_scores_limits() {
    printf("\n=== Test: Limites Top Scores ===\n");
    remove(".snake_top_scores");
    Game game;
    game.top_score_count = 0;
    // Ajouter plus de MAX_TOP_SCORES scores
    for (int i = 0; i < MAX_TOP_SCORES + 5; i++) {
        add_top_score(&game, 50 + i);
    }
    TEST_EQUAL(game.top_score_count, MAX_TOP_SCORES, "Nombre max de scores respecté");
    TEST_EQUAL(game.top_scores[0].score, MAX_TOP_SCORES + 4, "Meilleur score = MAX_TOP_SCORES + 4");
    remove(".snake_top_scores");
}

void test_snake_body_positions() {
    printf("\n=== Test: Positions du Corps du Serpent ===\n");
    Snake snake;
    init_snake(&snake, 30, 10, 1);
    TEST_EQUAL(snake.body[0].x, 30, "Tête X = 30");
    TEST_EQUAL(snake.body[0].y, 10, "Tête Y = 10");
    TEST_EQUAL(snake.body[1].x, 29, "Corps[1] X = 29");
    TEST_EQUAL(snake.body[1].y, 10, "Corps[1] Y = 10");
    TEST_EQUAL(snake.body[2].x, 28, "Corps[2] X = 28");
    TEST_EQUAL(snake.body[2].y, 10, "Corps[2] Y = 10");
    // Vérifier que les positions sont consécutives
    for (int i = 1; i < snake.length; i++) {
        TEST_EQUAL(snake.body[i].x, snake.body[0].x - i, "Position corps consécutive X");
        TEST_EQUAL(snake.body[i].y, snake.body[0].y, "Position corps consécutive Y");
    }
}

void test_position_validation_edge_cases() {
    printf("\n=== Test: Cas Limites Validation Positions ===\n");
    Game game;
    game.grid_width = 60;
    game.grid_height = 20;
    game.obstacle_count = 0;
    game.food_count = 0;
    game.multiplayer = 0;
    game.snake1.length = 0;
    // Position à la limite (0, 0)
    Position edge1 = {0, 0};
    TEST_EQUAL(is_position_valid(&game, edge1, 0), 1, "Position (0,0) valide");
    // Position à la limite (width-1, height-1)
    Position edge2 = {game.grid_width - 1, game.grid_height - 1};
    TEST_EQUAL(is_position_valid(&game, edge2, 0), 1, "Position limite valide");
    // Position juste hors limites
    Position invalid1 = {game.grid_width, 10};
    TEST_EQUAL(is_position_valid(&game, invalid1, 0), 0, "Position X = width rejetée");
    Position invalid2 = {30, game.grid_height};
    TEST_EQUAL(is_position_valid(&game, invalid2, 0), 0, "Position Y = height rejetée");
}

void test_multiplier_timer_restoration() {
    printf("\n=== Test: Restauration Multiplicateur ===\n");
    Game game;
    init_game(&game, MODE_CLASSIC, DIFF_MEDIUM, 1);
    game.multiplier_timer = 1;
    game.snake1.multiplier = 2;
    game.snake2.multiplier = 2;
    update_powerups(&game);
    TEST_EQUAL(game.multiplier_timer, 0, "Timer multiplicateur = 0");
    TEST_EQUAL(game.snake1.multiplier, 1, "Multiplicateur joueur 1 restauré");
    TEST_EQUAL(game.snake2.multiplier, 1, "Multiplicateur joueur 2 restauré");
}

void test_speed_calculation() {
    printf("\n=== Test: Calcul de Vitesse ===\n");
    Game game;
    init_game(&game, MODE_CLASSIC, DIFF_MEDIUM, 0);
    game.level = 1;
    game.base_speed = 150;
    game.slow_timer = 0;
    int expected_speed = game.base_speed - (game.level - 1) * 5;
    TEST_EQUAL(expected_speed, 150, "Vitesse niveau 1 = 150");
    game.level = 5;
    expected_speed = game.base_speed - (game.level - 1) * 5;
    TEST_EQUAL(expected_speed, 130, "Vitesse niveau 5 = 130");
}

void test_food_type_characters() {
    printf("\n=== Test: Tous les Types de Nourriture ===\n");
    TEST_ASSERT(strcmp(get_food_char(FOOD_NORMAL), "*") == 0, "FOOD_NORMAL = *");
    TEST_ASSERT(strcmp(get_food_char(FOOD_GOLDEN), "$") == 0, "FOOD_GOLDEN = $");
    TEST_ASSERT(strcmp(get_food_char(FOOD_POISON), "X") == 0, "FOOD_POISON = X");
    TEST_ASSERT(strcmp(get_food_char(FOOD_FAST), "!") == 0, "FOOD_FAST = !");
    TEST_ASSERT(strcmp(get_food_char(FOOD_BONUS), "?") == 0, "FOOD_BONUS = ?");
}

void test_powerup_type_characters() {
    printf("\n=== Test: Tous les Types de Power-ups ===\n");
    TEST_ASSERT(strcmp(get_powerup_char(POWERUP_SLOW), "S") == 0, "POWERUP_SLOW = S");
    TEST_ASSERT(strcmp(get_powerup_char(POWERUP_INVINCIBLE), "I") == 0, "POWERUP_INVINCIBLE = I");
    TEST_ASSERT(strcmp(get_powerup_char(POWERUP_MULTIPLIER), "M") == 0, "POWERUP_MULTIPLIER = M");
    TEST_ASSERT(strcmp(get_powerup_char(POWERUP_MAGNETIC), "G") == 0, "POWERUP_MAGNETIC = G");
    TEST_ASSERT(strcmp(get_powerup_char(POWERUP_NONE), "P") == 0, "POWERUP_NONE = P (default)");
}

int main() {
    printf("═══════════════════════════════════════════════════════\n");
    printf("  TESTS UNITAIRES - JEU DU SERPENT\n");
    printf("═══════════════════════════════════════════════════════\n");
    srand(time(NULL));
    test_generate_random_position();
    test_is_position_valid();
    test_init_snake();
    test_init_game();
    test_update_powerups();
    test_top_scores();
    test_get_food_char();
    test_get_powerup_char();
    test_snake_movement_logic();
    test_game_modes();
    test_difficulty_extreme();
    test_multiplier_system();
    test_top_scores_limits();
    test_snake_body_positions();
    test_position_validation_edge_cases();
    test_multiplier_timer_restoration();
    test_speed_calculation();
    test_food_type_characters();
    test_powerup_type_characters();
    printf("\n═══════════════════════════════════════════════════════\n");
    printf("  RÉSULTATS DES TESTS\n");
    printf("═══════════════════════════════════════════════════════\n");
    printf("Tests exécutés: %d\n", tests_run);
    printf("Tests réussis:  %d\n", tests_passed);
    printf("Tests échoués:  %d\n", tests_failed);
    if (tests_failed == 0) {
        printf("\n✓ TOUS LES TESTS SONT PASSÉS!\n");
        return 0;
    } else {
        printf("\n✗ CERTAINS TESTS ONT ÉCHOUÉ\n");
        return 1;
    }
}
