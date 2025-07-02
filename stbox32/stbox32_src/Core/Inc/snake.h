#ifndef SNAKE_H
#define SNAKE_H

#include <stdint.h>
#include <stdbool.h>

// Game constants
#define SNAKE_CELL_SIZE 8
#define SNAKE_GRID_WIDTH (LCD_WIDTH / SNAKE_CELL_SIZE)
#define SNAKE_GRID_HEIGHT (LCD_HEIGHT / SNAKE_CELL_SIZE)
#define SNAKE_MAX_LENGTH 200
#define SNAKE_INITIAL_LENGTH 3

// Colors
#define SNAKE_BACKGROUND_COLOR GREEN
#define SNAKE_BODY_COLOR 0xFFE0
#define SNAKE_APPLE_COLOR RED
#define SNAKE_BORDER_COLOR WHITE
#define SNAKE_GAME_OVER_COLOR RED

// Direction constants
#define SNAKE_DIR_NONE  0
#define SNAKE_DIR_UP    1
#define SNAKE_DIR_DOWN  2
#define SNAKE_DIR_LEFT  3
#define SNAKE_DIR_RIGHT 4

typedef struct {
    int x;
    int y;
} SnakePoint;

typedef struct {
    SnakePoint body[SNAKE_MAX_LENGTH];
    int length;
    int direction;
    int next_direction;
    bool game_over;
    int score;
} Snake;

typedef struct {
    int x;
    int y;
    bool exists;
} Apple;

// Function declarations
void snake_init(void);
void snake_play(void);
void snake_show_menu(void);
bool snake_wait_for_start(void);
void snake_reset_game(void);
void snake_draw_game(void);
void snake_draw_cell(int x, int y, uint16_t color);
void snake_draw_apple(void);
void snake_draw_border(void);
void snake_update(void);
void snake_move(void);
void snake_spawn_apple(void);
bool snake_check_collision(void);
bool snake_check_apple_collision(void);
void snake_handle_input(void);
void snake_show_game_over(void);

#endif
