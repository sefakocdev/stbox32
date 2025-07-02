#ifndef BREAKOUT_H
#define BREAKOUT_H


#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define PADDLE_WIDTH 40
#define PADDLE_HEIGHT 6
#define PADDLE_Y (LCD_HEIGHT - 20)

#define BALL_SIZE 6

#define BRICK_ROWS 5
#define BRICK_COLS 8
#define BRICK_WIDTH (LCD_WIDTH / BRICK_COLS)
#define BRICK_HEIGHT 12

#define BRICK_COLOR_COUNT 5

typedef struct {
    int x, y;
    int dx, dy; // yatay ve dikey hız
} Ball;

typedef struct {
    int x;
    int speed;
    int prev_paddle_x;
} Paddle;

typedef struct {
    int x;
    int y;
    int active;
    uint16_t color;
} Brick;

void breakout_init(int pSpeed);
void breakout_play(int pSpeed);
int breakout_update();
void breakout_won(void);
void breakout_game_over(void);
void breakout_draw(void);

#endif // BREAKOUT_H
