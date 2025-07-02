#ifndef PONG_H
#define PONG_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "main.h" // Include for HAL functions

// Game settings
#define PONG_PADDLE_WIDTH 40
#define PONG_PADDLE_HEIGHT 8
#define PONG_PADDLE_SPEED 20

#define PONG_BALL_SIZE 6
#define PONG_BALL_SPEED_X 15
#define PONG_BALL_SPEED_Y 15

#define PONG_TOP_PADDLE_Y 10
#define PONG_BOTTOM_PADDLE_Y (LCD_HEIGHT - PONG_TOP_PADDLE_Y - PONG_PADDLE_HEIGHT)

#define SCORE_TO_WIN 5

// Note frequencies for Pong background music (in Hz)
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_G5  784
#define NOTE_A5  880
#define NOTE_B5  988
#define NOTE_C6  1047
#define PONG_REST 0

typedef struct {
    int x, y;
    int dx, dy; // horizontal and vertical velocity
    int prev_x, prev_y; // previous position for clearing
} PongBall;

typedef struct {
    int x, y;
    int speed;
    int prev_x; // previous position for clearing
} PongPaddle;

typedef struct {
    int left_score;
    int right_score;
    int game_over;
    int winner; // 1 for left player, 2 for right player
} PongGame;

// Function declarations
void pong_init(void);
void pong_play(void);
int pong_update(void);
void pong_draw(void);
void pong_game_over(void);
void pong_reset_ball(void);
void pong_reset_paddles(void);
void pong_clear_paddle_lines(void);
void playPongSound(void);
void pongSound_start(void);
void pongSound_stop(void);
void pongSound_update(void);
void pongSound_playScore(void);
void pongSound_playWinner(void);

// Utility function
void WaitUntil(unsigned int ms);

#endif // PONG_H
