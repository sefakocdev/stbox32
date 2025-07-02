#ifndef GAME1942_H
#define GAME1942_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "main.h"
#include "lcd_driver.h"
#include "joystick.h"
#include "playmariosound.h"  // Include sound functionality

// Game constants
#define PLAYER_WIDTH 16
#define PLAYER_HEIGHT 20
#define PLAYER_SPEED 8
#define PLAYER_START_X (LCD_WIDTH / 2 - PLAYER_WIDTH / 2)
#define PLAYER_START_Y (LCD_HEIGHT - PLAYER_HEIGHT - 10)

#define BULLET_WIDTH 2
#define BULLET_HEIGHT 6
#define BULLET_SPEED 15
#define MAX_BULLETS 10

#define ENEMY_COLOR YELLOW
#define MAX_ENEMIES 8

// Game colors
#define GAME1942_BACKGROUND BLUE
#define PLAYER_COLOR GREEN
#define BULLET_COLOR RED

// Enemy movement bounding box (invisible)
#define ENEMY_BOUNDS_LEFT 10
#define ENEMY_BOUNDS_RIGHT (LCD_WIDTH - 10)
#define ENEMY_BOUNDS_TOP 50
#define ENEMY_BOUNDS_BOTTOM (LCD_HEIGHT - 50)

// Game structures
typedef struct {
    int x, y;
    int prev_x, prev_y;
    bool active;
} Player;

typedef struct {
    int x, y;
    int prev_x, prev_y;
    bool active;
} Bullet;

typedef enum {
    ENEMY_MOVE_STRAIGHT,
    ENEMY_MOVE_ZIGZAG,
    ENEMY_MOVE_RANDOM,
    ENEMY_MOVE_HORIZONTAL_LR, // Left to right
    ENEMY_MOVE_HORIZONTAL_RL  // Right to left
} EnemyMovePattern;

typedef struct {
    int x, y;
    int prev_x, prev_y;
    int speed;
    EnemyMovePattern pattern;
    int zigzag_dir; // 1 for right, -1 for left
    int zigzag_counter;
    bool active;
} Enemy;

typedef struct {
    Player player;
    Bullet bullets[MAX_BULLETS];
    Enemy enemies[MAX_ENEMIES];
    int score;
    int lives;
    bool game_over;
    unsigned int enemy_spawn_interval;
    unsigned int last_enemy_spawn_time;
} Game1942;

// Function declarations
void game1942_init(void);
void game1942_play(void);
void game1942_update(void);
void game1942_draw(void);
void game1942_clear_previous(void);

// Player functions
void player_update(void);
void player_draw(void);
void player_clear(void);
void player_fire(void);

// Bullet functions
void bullets_update(void);
void bullets_draw(void);
void bullets_clear(void);

// Enemy functions
void enemies_init(void);
void enemies_update(void);
void enemies_draw(void);
void enemies_spawn(void);
void enemies_clear(void);
void clear_enemy_shape(int x, int y);

// UI functions
void init_ui(void);
void draw_game_over(void);

// Sound functions
void play_bullet_fire_sound(void);

#endif // GAME1942_H
