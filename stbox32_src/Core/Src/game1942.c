#include "game1942.h"
#include "pong.h"  // For WaitUntil function
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "stm32f4xx_hal.h"
#include "playmariosound.h"  // Include sound functionality

// Define M_PI if not already defined to fix the compile error for undefined M_PI in trigonometric calculations
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Global game state
static Game1942 game;

// Dynamic limits for enemies and bullets
static int max_enemies = 5;
static int max_bullets = 5;

#define ENEMY_BULLET_COLOR MAGENTA
#define MAX_ENEMY_BULLETS 20

typedef struct {
    int x, y;
    int prev_x, prev_y;
    int active;
} EnemyBullet;

static EnemyBullet enemy_bullets[MAX_ENEMY_BULLETS];
static unsigned int last_enemy_bullet_time = 0;
#define ENEMY_BULLET_SPEED 7

// Function prototype to avoid implicit declaration warning
void enemy_bullets_update(void);
void player_explosion_effect(int x, int y);
void short_explosion_effect(int x, int y); // Short explosion for bullet-enemy

// Timing variables
static unsigned int last_update_time = 0;
static unsigned int last_fire_time = 0;

// Initialize game
void game1942_init(void)
{
    // Set proper rotation for horizontal gameplay
    setRotation(1); // Horizontal orientation
    
    lcd_clear_screen(GAME1942_BACKGROUND);
    
    // Initialize player
    game.player.x = PLAYER_START_X;
    game.player.y = PLAYER_START_Y;
    game.player.prev_x = PLAYER_START_X;
    game.player.prev_y = PLAYER_START_Y;
    game.player.active = true;
      // Initialize bullets
    for (int i = 0; i < max_bullets; i++) {
        game.bullets[i].active = false;
        game.bullets[i].x = 0;
        game.bullets[i].y = 0;
        game.bullets[i].prev_x = 0;
        game.bullets[i].prev_y = 0;
    }
    
    // Initialize enemies
    for (int i = 0; i < max_enemies; i++) {
        game.enemies[i].active = false;
    }
    game.enemy_spawn_interval = 1500; // Start with 1.5 seconds
    game.last_enemy_spawn_time = HAL_GetTick();
    
    // Initialize game state
    game.score = 0;
    game.lives = 3;
    game.game_over = false;

    // Initialize enemy bullets
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        enemy_bullets[i].active = 0;
        enemy_bullets[i].x = 0;
        enemy_bullets[i].y = 0;
        enemy_bullets[i].prev_x = 0;
        enemy_bullets[i].prev_y = 0;
    }
    last_enemy_bullet_time = HAL_GetTick();
    
    init_ui();         // UI always on top
}

// Main game loop
void game1942_play(void)
{
    lcd_clear_screen(GAME1942_BACKGROUND);
      // Show game title
    lcd_display_string(80, 100, (const uint8_t *)"1942 FIGHTER", 12, GREEN);
    lcd_display_string(60, 180, (const uint8_t *)"Press Button To Start", 12, CYAN);
      // Wait for button press
    while (1) {
        if (Joystick_ReadButton(JOYSTICK_1) || Joystick_ReadButton(JOYSTICK_2))
            break;
        WaitUntil(50);
    }
      game1942_init();
    last_update_time = HAL_GetTick();
    // Main game loop with frame rate control
    while (!game.game_over) {
        unsigned int current_time = HAL_GetTick();
        
        // Update at ~30 FPS (33ms per frame)
        if (current_time - last_update_time >= 33) {
            game1942_update();
            game1942_draw();
            last_update_time = current_time;
        }
    }
    
    draw_game_over();
    WaitUntil(3000);
}

// Collision detection helper
static bool rects_overlap(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
    return !(x1 + w1 < x2 || x2 + w2 < x1 || y1 + h1 < y2 || y2 + h2 < y1);
}

// Update game state
void game1942_update(void)
{
    player_update();
    bullets_update();
    enemies_spawn();
    enemies_update();
    enemy_bullets_update();

    // Bullet-enemy collision
    for (int i = 0; i < max_bullets; i++) {
        if (!game.bullets[i].active) continue;
        for (int j = 0; j < max_enemies; j++) {
            if (!game.enemies[j].active) continue;
            if (rects_overlap(
                    game.bullets[i].x, game.bullets[i].y, BULLET_WIDTH, BULLET_HEIGHT,
                    game.enemies[j].x, game.enemies[j].y, 12, 17)) {
                // Clear enemy at both current and previous positions
                clear_enemy_shape(game.enemies[j].x, game.enemies[j].y);
                clear_enemy_shape(game.enemies[j].prev_x, game.enemies[j].prev_y);
                // Clear bullet at both current and previous positions
                lcd_fill_rect(game.bullets[i].x, game.bullets[i].y, BULLET_WIDTH, BULLET_HEIGHT, GAME1942_BACKGROUND);
                lcd_fill_rect(game.bullets[i].prev_x, game.bullets[i].prev_y, BULLET_WIDTH, BULLET_HEIGHT, GAME1942_BACKGROUND);
                game.bullets[i].active = false;
                game.enemies[j].active = false;

                // Short explosion effect at enemy position
                short_explosion_effect(game.enemies[j].x + 6, game.enemies[j].y + 8); // Center of enemy

                // Update score display immediately after score change
                char score_str[32];
                sprintf(score_str, "SCORE: %d", game.score);
                lcd_display_string(15, 3, (const uint8_t *)score_str, 12, BLACK);
                game.score += 10;
                sprintf(score_str, "SCORE: %d", game.score);
                lcd_display_string(15, 3, (const uint8_t *)score_str, 12, WHITE);
                break;
            }
        }
    }



    // Enemy-player collision
    for (int j = 0; j < max_enemies; j++) {
        if (!game.enemies[j].active) continue;
        if (rects_overlap(
                game.player.x, game.player.y, PLAYER_WIDTH, PLAYER_HEIGHT,
                game.enemies[j].x, game.enemies[j].y, 12, 17)) {
            // Clear enemy at both current and previous positions
            clear_enemy_shape(game.enemies[j].x, game.enemies[j].y);
            clear_enemy_shape(game.enemies[j].prev_x, game.enemies[j].prev_y);
            game.enemies[j].active = false;
            player_explosion_effect(game.player.x, game.player.y);

            // Draw lives (right side)
            char lives_str[16];
            sprintf(lives_str, "LIVES: %d", game.lives);
            // Calculate width of text area, align right with 4px margin
            int lives_x = LCD_WIDTH - 4 - 8 * strlen(lives_str); // 8px per char, 4px margin
            if (lives_x < 0) lives_x = 0;
            lcd_display_string(lives_x, 3, (const uint8_t *)lives_str, 12, BLACK);

            game.lives--;

            // Draw lives (right side, yellow text)
            sprintf(lives_str, "LIVES: %d", game.lives);
            // Calculate width of text area, align right with 4px margin
            lives_x = LCD_WIDTH - 4 - 8 * strlen(lives_str); // 8px per char, 4px margin
            if (lives_x < 0) lives_x = 0;
            lcd_display_string(lives_x, 3, (const uint8_t *)lives_str, 12, WHITE);
            if (game.lives <= 0) {
                game.game_over = true;
            }
        }
    }

    // Enemy bullet collision with player
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (!enemy_bullets[i].active) continue;
        if (rects_overlap(game.player.x, game.player.y, PLAYER_WIDTH, PLAYER_HEIGHT,
                          enemy_bullets[i].x, enemy_bullets[i].y, BULLET_WIDTH, BULLET_HEIGHT)) {
            // Remove bullet and decrease life
            lcd_fill_rect(enemy_bullets[i].x, enemy_bullets[i].y, BULLET_WIDTH, BULLET_HEIGHT, GAME1942_BACKGROUND);
            lcd_fill_rect(enemy_bullets[i].prev_x, enemy_bullets[i].prev_y, BULLET_WIDTH, BULLET_HEIGHT, GAME1942_BACKGROUND);
            enemy_bullets[i].active = 0;
            player_explosion_effect(game.player.x, game.player.y);

            // Draw lives (right side, yellow text)
			char lives_str[16];
			sprintf(lives_str, "LIVES: %d", game.lives);
			// Calculate width of text area, align right with 4px margin
			int lives_x = LCD_WIDTH - 4 - 8 * strlen(lives_str); // 8px per char, 4px margin
			if (lives_x < 0) lives_x = 0;
			lcd_display_string(lives_x, 3, (const uint8_t *)lives_str, 12, BLACK);

			game.lives--;

			// Draw lives (right side, yellow text)
			sprintf(lives_str, "LIVES: %d", game.lives);
			// Calculate width of text area, align right with 4px margin
			lives_x = LCD_WIDTH - 4 - 8 * strlen(lives_str); // 8px per char, 4px margin
			if (lives_x < 0) lives_x = 0;
			lcd_display_string(lives_x, 3, (const uint8_t *)lives_str, 12, WHITE);
			if (game.lives <= 0) {
				game.game_over = true;
			}
            if (game.lives <= 0) game.game_over = true;
        }
    }

    // Check game over condition
    if (game.lives <= 0) {
        game.game_over = true;
    }

    // Increase max_enemies and max_bullets at 100 points
    if (game.score >= 100) {
        max_enemies = 5 + (game.score/100);
        max_bullets = 10;
        // Initialize new slots
        for (int i = 5; i < max_bullets; i++) {
            game.bullets[i].active = false;
            game.bullets[i].x = 0;
            game.bullets[i].y = 0;
            game.bullets[i].prev_x = 0;
            game.bullets[i].prev_y = 0;
        }
        for (int i = 5; i < max_enemies; i++) {
            game.enemies[i].active = false;
        }
    }
}

// Draw everything
void game1942_draw(void)
{
    game1942_clear_previous();
    // Draw enemies first (behind player)
    enemies_draw();
    bullets_draw();    // Draw bullets
    player_draw();     // Draw player
}

// Clear previous positions
void game1942_clear_previous(void)
{
    enemies_clear();
    bullets_clear();
    
    // Clear player previous position only if it moved
    if (game.player.prev_x != game.player.x || game.player.prev_y != game.player.y) {
        player_clear();
    }
}

// Player functions
void player_update(void)
{
    if (!game.player.active) return;
    
    // Store previous position
    game.player.prev_x = game.player.x;
    game.player.prev_y = game.player.y;
      // Read joystick direction
    int direction = Joystick_ReadDirection(JOYSTICK_1);
    
    // Move player based on joystick direction with custom mapping:
    // LEFT = move UP, RIGHT = move DOWN, UP = move RIGHT, DOWN = move LEFT
    
    // Vertical movement (LEFT/RIGHT joystick controls UP/DOWN movement)
    if ((direction == JOY_LEFT || direction == JOY_UP_LEFT || direction == JOY_DOWN_LEFT) && game.player.y > LCD_HEIGHT / 2) {
        game.player.y -= PLAYER_SPEED;
        if (game.player.y < LCD_HEIGHT / 2) game.player.y = LCD_HEIGHT / 2;
    }
    if ((direction == JOY_RIGHT || direction == JOY_UP_RIGHT || direction == JOY_DOWN_RIGHT) && game.player.y + PLAYER_HEIGHT < LCD_HEIGHT) {
        game.player.y += PLAYER_SPEED;
        if (game.player.y + PLAYER_HEIGHT > LCD_HEIGHT)
            game.player.y = LCD_HEIGHT - PLAYER_HEIGHT;
    }
    
    // Horizontal movement (UP/DOWN joystick controls RIGHT/LEFT movement)
    if ((direction == JOY_UP || direction == JOY_UP_LEFT || direction == JOY_UP_RIGHT) && game.player.x + PLAYER_WIDTH < LCD_WIDTH) {
        game.player.x += PLAYER_SPEED;
        if (game.player.x + PLAYER_WIDTH > LCD_WIDTH) 
            game.player.x = LCD_WIDTH - PLAYER_WIDTH;
    }
    if ((direction == JOY_DOWN || direction == JOY_DOWN_LEFT || direction == JOY_DOWN_RIGHT) && game.player.x > 0) {
        game.player.x -= PLAYER_SPEED;
        if (game.player.x < 0) game.player.x = 0;
    }
    
    // Fire bullet on button press with rate limiting
    unsigned int current_time = HAL_GetTick();
    if (Joystick_ReadButton(JOYSTICK_1) && (current_time - last_fire_time >= 100)) {
        player_fire();
        last_fire_time = current_time;
    }
    else {
        unsigned int start = HAL_GetTick();
        while ((HAL_GetTick() - start) < 60) {
        }

    }
}

void player_draw(void)
{
    if (!game.player.active) return;
    
    // Draw enhanced plane shape for bigger size
    int x = game.player.x;
    int y = game.player.y;
    
    // Main fuselage (thicker for larger plane)
    lcd_draw_line(x + PLAYER_WIDTH/2 - 1, y, x + PLAYER_WIDTH/2 - 1, y + PLAYER_HEIGHT, PLAYER_COLOR);
    lcd_draw_line(x + PLAYER_WIDTH/2, y, x + PLAYER_WIDTH/2, y + PLAYER_HEIGHT, PLAYER_COLOR);
    lcd_draw_line(x + PLAYER_WIDTH/2 + 1, y, x + PLAYER_WIDTH/2 + 1, y + PLAYER_HEIGHT, PLAYER_COLOR);
    
    // Main wings (wider and more prominent)
    lcd_draw_line(x, y + PLAYER_HEIGHT/3, x + PLAYER_WIDTH, y + PLAYER_HEIGHT/3, PLAYER_COLOR);
    lcd_draw_line(x + 1, y + PLAYER_HEIGHT/3 + 1, x + PLAYER_WIDTH - 1, y + PLAYER_HEIGHT/3 + 1, PLAYER_COLOR);
    
    // Secondary wings (smaller, upper wings)
    lcd_draw_line(x + PLAYER_WIDTH/4, y + PLAYER_HEIGHT/5, x + 3*PLAYER_WIDTH/4, y + PLAYER_HEIGHT/5, PLAYER_COLOR);
    
    // Tail wings
    lcd_draw_line(x + PLAYER_WIDTH/3, y + PLAYER_HEIGHT - 2, x + 2*PLAYER_WIDTH/3, y + PLAYER_HEIGHT - 2, PLAYER_COLOR);
    lcd_draw_line(x + PLAYER_WIDTH/4, y + PLAYER_HEIGHT, x + 3*PLAYER_WIDTH/4, y + PLAYER_HEIGHT, PLAYER_COLOR);
    
    // Nose/cockpit (enhanced with multiple points)
    lcd_draw_point(x + PLAYER_WIDTH/2 - 1, y, GREEN);
    lcd_draw_point(x + PLAYER_WIDTH/2, y, GREEN);
    lcd_draw_point(x + PLAYER_WIDTH/2 + 1, y, GREEN);
    lcd_draw_point(x + PLAYER_WIDTH/2, y + 1, YELLOW);
}

void player_clear(void)
{
    // Clear previous position - enhanced clearing to match enhanced drawing
    int x = game.player.prev_x;
    int y = game.player.prev_y;
    
    // Clear main fuselage (thicker)
    lcd_draw_line(x + PLAYER_WIDTH/2 - 1, y, x + PLAYER_WIDTH/2 - 1, y + PLAYER_HEIGHT, GAME1942_BACKGROUND);
    lcd_draw_line(x + PLAYER_WIDTH/2, y, x + PLAYER_WIDTH/2, y + PLAYER_HEIGHT, GAME1942_BACKGROUND);
    lcd_draw_line(x + PLAYER_WIDTH/2 + 1, y, x + PLAYER_WIDTH/2 + 1, y + PLAYER_HEIGHT, GAME1942_BACKGROUND);
    
    // Clear main wings
    lcd_draw_line(x, y + PLAYER_HEIGHT/3, x + PLAYER_WIDTH, y + PLAYER_HEIGHT/3, GAME1942_BACKGROUND);
    lcd_draw_line(x + 1, y + PLAYER_HEIGHT/3 + 1, x + PLAYER_WIDTH - 1, y + PLAYER_HEIGHT/3 + 1, GAME1942_BACKGROUND);
    
    // Clear secondary wings
    lcd_draw_line(x + PLAYER_WIDTH/4, y + PLAYER_HEIGHT/5, x + 3*PLAYER_WIDTH/4, y + PLAYER_HEIGHT/5, GAME1942_BACKGROUND);
    
    // Clear tail wings
    lcd_draw_line(x + PLAYER_WIDTH/3, y + PLAYER_HEIGHT - 2, x + 2*PLAYER_WIDTH/3, y + PLAYER_HEIGHT - 2, GAME1942_BACKGROUND);
    lcd_draw_line(x + PLAYER_WIDTH/4, y + PLAYER_HEIGHT, x + 3*PLAYER_WIDTH/4, y + PLAYER_HEIGHT, GAME1942_BACKGROUND);
    
    // Clear nose/cockpit
    lcd_draw_point(x + PLAYER_WIDTH/2 - 1, y, GAME1942_BACKGROUND);
    lcd_draw_point(x + PLAYER_WIDTH/2, y, GAME1942_BACKGROUND);
    lcd_draw_point(x + PLAYER_WIDTH/2 + 1, y, GAME1942_BACKGROUND);
    lcd_draw_point(x + PLAYER_WIDTH/2, y + 1, GAME1942_BACKGROUND);
}

void player_fire(void)
{
    // Double bullets only after 200 points
    int bullets_to_fire = (game.score >= 200) ? 2 : 1;
    int bullets_fired = 0;
    int left_offset = -PLAYER_WIDTH/4;
    int right_offset = PLAYER_WIDTH/4;

    for (int i = 0; i < max_bullets && bullets_fired < bullets_to_fire; i++) {
        if (!game.bullets[i].active) {
            if (bullets_to_fire == 1) {
                // Single bullet (center)
                game.bullets[i].x = game.player.x + PLAYER_WIDTH/2 - BULLET_WIDTH/2;
                game.bullets[i].y = game.player.y - BULLET_HEIGHT;
            } else {
                // Double bullets: left and right of center
                if (bullets_fired == 0) {
                    // Left bullet
                    game.bullets[i].x = game.player.x + PLAYER_WIDTH/2 - BULLET_WIDTH/2 + left_offset;
                    game.bullets[i].y = game.player.y - BULLET_HEIGHT;
                } else {
                    // Right bullet
                    game.bullets[i].x = game.player.x + PLAYER_WIDTH/2 - BULLET_WIDTH/2 + right_offset;
                    game.bullets[i].y = game.player.y - BULLET_HEIGHT;
                }
            }
            game.bullets[i].prev_x = game.bullets[i].x;
            game.bullets[i].prev_y = game.bullets[i].y;
            game.bullets[i].active = true;
            bullets_fired++;
            if (bullets_fired == 1) {
                play_bullet_fire_sound();
            }
        }
    }
}

// Bullet functions
void bullets_update(void)
{
    for (int i = 0; i < max_bullets; i++) {
        if (!game.bullets[i].active) continue;
        
        // Store previous position
        game.bullets[i].prev_x = game.bullets[i].x;
        game.bullets[i].prev_y = game.bullets[i].y;
          // Move bullet up
        game.bullets[i].y -= BULLET_SPEED;
        
        // Deactivate if off screen
        if (game.bullets[i].y < 15) {  // Changed from 10 to 15 to match taller UI bar
            // Clear the bullet from screen before deactivating
            lcd_fill_rect(game.bullets[i].prev_x, game.bullets[i].prev_y, BULLET_WIDTH, BULLET_HEIGHT, GAME1942_BACKGROUND);
            game.bullets[i].active = false;
        }
    }
}

void bullets_draw(void)
{
    for (int i = 0; i < max_bullets; i++) {
        if (!game.bullets[i].active) continue;
        
        // Draw bullet as small rectangle
        lcd_fill_rect(game.bullets[i].x, game.bullets[i].y, BULLET_WIDTH, BULLET_HEIGHT, BULLET_COLOR);
    }
    // Draw enemy bullets (magenta)
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (!enemy_bullets[i].active) continue;
        lcd_fill_rect(enemy_bullets[i].x, enemy_bullets[i].y, BULLET_WIDTH, BULLET_HEIGHT, ENEMY_BULLET_COLOR);
    }
}

void bullets_clear(void)
{
    for (int i = 0; i < max_bullets; i++) {
        if (!game.bullets[i].active) continue;
        
        // Only clear if the bullet has actually moved
        if (game.bullets[i].prev_x == game.bullets[i].x && 
            game.bullets[i].prev_y == game.bullets[i].y) {
            continue;
        }
        
        // Clear previous position
        lcd_fill_rect(game.bullets[i].prev_x, game.bullets[i].prev_y, BULLET_WIDTH, BULLET_HEIGHT, GAME1942_BACKGROUND);
    }
    // Clear enemy bullets
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (!enemy_bullets[i].active) continue;
        if (enemy_bullets[i].prev_x == enemy_bullets[i].x && enemy_bullets[i].prev_y == enemy_bullets[i].y) continue;
        lcd_fill_rect(enemy_bullets[i].prev_x, enemy_bullets[i].prev_y, BULLET_WIDTH, BULLET_HEIGHT, GAME1942_BACKGROUND);
    }
}

// Enemy management functions
void enemies_init(void) {
    for (int i = 0; i < max_enemies; i++) {
        game.enemies[i].active = false;
    }
    game.enemy_spawn_interval = 1500; // Start with 1.5 seconds
    game.last_enemy_spawn_time = HAL_GetTick();
}

void spawn_enemy_formation(int pattern) {
    int available = 0;
    for (int i = 0; i < max_enemies; i++) if (!game.enemies[i].active) available++;
    if (available < 1) return;
    for (int j = 0; j < max_enemies; j++) {
        if (!game.enemies[j].active) {
            int x, y, speed, zigzag_dir = 0;
            switch (pattern) {
                case ENEMY_MOVE_STRAIGHT:
                    x = 20 + rand() % (LCD_WIDTH - 40);
                    y = 15;
                    speed = 5 + rand() % 2;
                    break;
                case ENEMY_MOVE_ZIGZAG:
                    x = 20 + rand() % (LCD_WIDTH - 40);
                    y = 15;
                    speed = 5;
                    zigzag_dir = (rand() % 2) ? 1 : -1;
                    break;
                case ENEMY_MOVE_HORIZONTAL_LR:
                    x = 0;
                    y = 30 + rand() % (LCD_HEIGHT/2 - 40);
                    speed = 5;
                    break;
                default:
                    x = 20 + rand() % (LCD_WIDTH - 40);
                    y = 15;
                    speed = 5;
                    break;
            }
            game.enemies[j].x = x;
            game.enemies[j].y = y;
            game.enemies[j].prev_x = x;
            game.enemies[j].prev_y = y;
            game.enemies[j].speed = speed;
            game.enemies[j].pattern = pattern;
            game.enemies[j].zigzag_dir = zigzag_dir;
            game.enemies[j].zigzag_counter = 0;
            game.enemies[j].active = true;
            break;
        }
    }
}

void enemies_spawn(void) {
    unsigned int now = HAL_GetTick();
    if (now - game.last_enemy_spawn_time < game.enemy_spawn_interval) return;
    int pattern = rand() % 4;
    spawn_enemy_formation(pattern);
    if (game.enemy_spawn_interval > 400) game.enemy_spawn_interval -= 10;
    game.last_enemy_spawn_time = now;
}

void enemies_update(void) {
    for (int i = 0; i < max_enemies; i++) {
        if (!game.enemies[i].active) continue;
        game.enemies[i].prev_x = game.enemies[i].x;
        game.enemies[i].prev_y = game.enemies[i].y;
        switch (game.enemies[i].pattern) {
            case ENEMY_MOVE_STRAIGHT:
                game.enemies[i].y += game.enemies[i].speed;
                break;
            case ENEMY_MOVE_ZIGZAG:
                game.enemies[i].y += game.enemies[i].speed;
                game.enemies[i].x += game.enemies[i].zigzag_dir * 2;
                // Reverse direction if hitting left/right bounds
                if (game.enemies[i].x <= ENEMY_BOUNDS_LEFT || game.enemies[i].x >= ENEMY_BOUNDS_RIGHT - 12) {
                    game.enemies[i].zigzag_dir *= -1;
                }
                game.enemies[i].zigzag_counter++;
                if (game.enemies[i].zigzag_counter > 20) {
                    game.enemies[i].zigzag_dir *= -1;
                    game.enemies[i].zigzag_counter = 0;
                }
                break;
            case ENEMY_MOVE_RANDOM:
                game.enemies[i].y += game.enemies[i].speed;
                game.enemies[i].x += (rand() % 3) - 1;
                // Reverse direction if hitting left/right bounds
                if (game.enemies[i].x <= ENEMY_BOUNDS_LEFT || game.enemies[i].x >= ENEMY_BOUNDS_RIGHT - 12) {
                    game.enemies[i].x = (game.enemies[i].x <= ENEMY_BOUNDS_LEFT) ? ENEMY_BOUNDS_LEFT : ENEMY_BOUNDS_RIGHT - 12;
                }
                break;
            case ENEMY_MOVE_HORIZONTAL_LR:
                game.enemies[i].x += game.enemies[i].speed;
                // Reverse direction if hitting left/right bounds
                if (game.enemies[i].x >= ENEMY_BOUNDS_RIGHT - 12) {
                    game.enemies[i].x = ENEMY_BOUNDS_RIGHT - 12;
                    game.enemies[i].speed *= -1;
                }
                if (game.enemies[i].x <= ENEMY_BOUNDS_LEFT) {
                    game.enemies[i].x = ENEMY_BOUNDS_LEFT;
                    game.enemies[i].speed *= -1;
                }
                break;
            case ENEMY_MOVE_HORIZONTAL_RL:
                game.enemies[i].x -= game.enemies[i].speed;
                // Reverse direction if hitting left/right bounds
                if (game.enemies[i].x <= ENEMY_BOUNDS_LEFT) {
                    game.enemies[i].x = ENEMY_BOUNDS_LEFT;
                    game.enemies[i].speed *= -1;
                }
                if (game.enemies[i].x >= ENEMY_BOUNDS_RIGHT - 12) {
                    game.enemies[i].x = ENEMY_BOUNDS_RIGHT - 12;
                    game.enemies[i].speed *= -1;
                }
                break;
        }
        // Deactivate if off screen or reached bottom
        int enemy_bottom = game.enemies[i].y + 17; // 17 = enemy height
        int next_enemy_bottom = game.enemies[i].y + game.enemies[i].speed + 17;
        if (game.enemies[i].x < -20 || game.enemies[i].x > LCD_WIDTH+20 || enemy_bottom > LCD_HEIGHT+20) {
            clear_enemy_shape(game.enemies[i].x, game.enemies[i].y);
            clear_enemy_shape(game.enemies[i].prev_x, game.enemies[i].prev_y);
            game.enemies[i].active = false;
        } else if (enemy_bottom >= LCD_HEIGHT || next_enemy_bottom >= LCD_HEIGHT) {
            // Enemy reached or will reach bottom: clear, deactivate, and decrease a life
            clear_enemy_shape(game.enemies[i].x, game.enemies[i].y);
            clear_enemy_shape(game.enemies[i].prev_x, game.enemies[i].prev_y);
            game.enemies[i].active = false;
            player_explosion_effect(game.player.x, game.player.y);
            // Draw lives (right side)
			char lives_str[16];
			sprintf(lives_str, "LIVES: %d", game.lives);
			// Calculate width of text area, align right with 4px margin
			int lives_x = LCD_WIDTH - 4 - 8 * strlen(lives_str); // 8px per char, 4px margin
			if (lives_x < 0) lives_x = 0;
			lcd_display_string(lives_x, 3, (const uint8_t *)lives_str, 12, BLACK);

			game.lives--;

			// Draw lives (right side, yellow text)
			sprintf(lives_str, "LIVES: %d", game.lives);
			// Calculate width of text area, align right with 4px margin
			lives_x = LCD_WIDTH - 4 - 8 * strlen(lives_str); // 8px per char, 4px margin
			if (lives_x < 0) lives_x = 0;
			lcd_display_string(lives_x, 3, (const uint8_t *)lives_str, 12, WHITE);
			if (game.lives <= 0) {
				game.game_over = true;
			}
            if (game.lives <= 0) {
                game.game_over = true;
            }
        }
    }
}

void enemies_draw(void) {
    for (int i = 0; i < max_enemies; i++) {
        if (!game.enemies[i].active) continue;
        int x = game.enemies[i].x;
        int y = game.enemies[i].y;
        // Enemy plane: thinner, longer fuselage, swept wings, tail fin, red nose
        // Fuselage
        lcd_draw_line(x+5, y, x+5, y+14, ENEMY_COLOR);
        lcd_draw_line(x+6, y, x+6, y+14, ENEMY_COLOR);
        // Wings (swept back)
        lcd_draw_line(x, y+7, x+11, y+11, ENEMY_COLOR); // left wing
        lcd_draw_line(x+11, y+7, x, y+11, ENEMY_COLOR); // right wing
        // Tail fin
        lcd_draw_line(x+5, y+14, x+8, y+17, ENEMY_COLOR);
        // Cockpit/nose (red)
        lcd_draw_point(x+5, y, RED);
        lcd_draw_point(x+6, y, RED);
    }
}

void clear_enemy_shape(int x, int y) {
    // Clear fuselage
    lcd_draw_line(x+5, y, x+5, y+14, GAME1942_BACKGROUND);
    lcd_draw_line(x+6, y, x+6, y+14, GAME1942_BACKGROUND);
    // Clear wings (thicker)
    for (int dy = 0; dy <= 1; dy++) {
        lcd_draw_line(x, y+7+dy, x+11, y+11+dy, GAME1942_BACKGROUND);
        lcd_draw_line(x+11, y+7+dy, x, y+11+dy, GAME1942_BACKGROUND);
    }
    // Clear tail fin (thicker)
    for (int dx = 0; dx <= 1; dx++) {
        lcd_draw_line(x+5+dx, y+14, x+8+dx, y+17, GAME1942_BACKGROUND);
    }
    // Clear cockpit/nose (bigger)
    for (int dx = 0; dx <= 1; dx++) {
        for (int dy = 0; dy <= 1; dy++) {
            lcd_draw_point(x+5+dx, y+dy, GAME1942_BACKGROUND);
        }
    }
}

void enemies_clear(void) {
    for (int i = 0; i < max_enemies; i++) {
        if (!game.enemies[i].active) continue;
        clear_enemy_shape(game.enemies[i].prev_x, game.enemies[i].prev_y);
    }
}

// UI functions
void init_ui(void)
{
    // Draw 15-pixel high black border at the top
    lcd_fill_rect(0, 0, LCD_WIDTH, 15, BLACK);

    // Draw score (left side, yellow text)
    char score_str[16];
    sprintf(score_str, "SCORE: %d", game.score);
    lcd_display_string(15, 3, (const uint8_t *)score_str, 12, WHITE);

    // Draw lives (right side, yellow text)
    char lives_str[16];
    sprintf(lives_str, "LIVES: %d", game.lives);
    // Calculate width of text area, align right with 4px margin
    int lives_x = LCD_WIDTH - 4 - 8 * strlen(lives_str); // 8px per char, 4px margin
    if (lives_x < 0) lives_x = 0;
    lcd_display_string(lives_x, 3, (const uint8_t *)lives_str, 12, WHITE);
}

void draw_game_over(void)
{
    lcd_clear_screen(GAME1942_BACKGROUND);
    
    lcd_display_string(80, 120, (const uint8_t *)"GAME OVER", 16, RED);
    
    char final_score[32];
    sprintf(final_score, "FINAL SCORE: %d", game.score);
    lcd_display_string(60, 160, (const uint8_t *)final_score, 12, YELLOW);
    
    lcd_display_string(50, 200, (const uint8_t *)"Press any button to return", 12, WHITE);
      // Wait for button press
    while (1) {
        if (Joystick_ReadButton(JOYSTICK_1) || Joystick_ReadButton(JOYSTICK_2))
            break;
        WaitUntil(50);
    }
}

// Sound functions
void play_bullet_fire_sound(void)
{
    // Play a short, sharp "pew" sound for bullet firing
    // High frequency for sharp shooting sound
    play_tone_conditional(1200, 50);  // 1.2kHz for 50ms - quick sharp sound
    play_tone_conditional(800, 30);   // Drop to 800Hz for 30ms - creates "pew" effect
}

void enemy_bullets_update(void) {
    // Fire enemy bullets every 5 seconds
    unsigned int now = HAL_GetTick();
    if (now - last_enemy_bullet_time >= 5000) {
        for (int j = 0; j < max_enemies; j++) {
            if (!game.enemies[j].active) continue;
            if (j % 4 != 0) continue; // Only 1 of each 4 planes can shoot
            // Find a free enemy bullet slot
            for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
                if (!enemy_bullets[i].active) {
                    enemy_bullets[i].x = game.enemies[j].x + 6; // center of enemy
                    enemy_bullets[i].y = game.enemies[j].y + 17; // bottom of enemy
                    enemy_bullets[i].prev_x = enemy_bullets[i].x;
                    enemy_bullets[i].prev_y = enemy_bullets[i].y;
                    enemy_bullets[i].active = 1;
                    break;
                }
            }
        }
        last_enemy_bullet_time = now;
    }
    // Move and clear enemy bullets
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (!enemy_bullets[i].active) continue;
        enemy_bullets[i].prev_x = enemy_bullets[i].x;
        enemy_bullets[i].prev_y = enemy_bullets[i].y;
        enemy_bullets[i].y += ENEMY_BULLET_SPEED;
        // Remove if off screen (bottom)
        int bullet_bottom = enemy_bullets[i].y + BULLET_HEIGHT;
        int next_bullet_bottom = enemy_bullets[i].y + ENEMY_BULLET_SPEED + BULLET_HEIGHT;
        if (bullet_bottom >= LCD_HEIGHT || next_bullet_bottom >= LCD_HEIGHT) {
            lcd_fill_rect(enemy_bullets[i].prev_x, enemy_bullets[i].prev_y, BULLET_WIDTH, BULLET_HEIGHT, GAME1942_BACKGROUND);
            enemy_bullets[i].active = 0;
        }
    }
}

// Explosion effect for player hit
void player_explosion_effect(int x, int y) {
    int cx = x + PLAYER_WIDTH/2;
    int cy = y + PLAYER_HEIGHT/2;
    // Clamp center to LCD bounds
    if (cx < 0) cx = 0;
    if (cy < 0) cy = 0;
    if (cx > LCD_WIDTH-1) cx = LCD_WIDTH-1;
    if (cy > LCD_HEIGHT-1) cy = LCD_HEIGHT-1;
    // Screen shake effect: shift explosion center rapidly
    for (int shake = 0; shake < 3; shake++) {
        int dx = (rand() % 7) - 3; // -3 to +3
        int dy = (rand() % 7) - 3;
        int scx = cx + dx;
        int scy = cy + dy;
        // Clamp shake center to LCD bounds
        if (scx < 0) scx = 0;
        if (scy < 0) scy = 0;
        if (scx > LCD_WIDTH-1) scx = LCD_WIDTH-1;
        if (scy > LCD_HEIGHT-1) scy = LCD_HEIGHT-1;
        // Draw multiple concentric circles (explosion)
        lcd_draw_circle(scx, scy, 12, RED);
        lcd_draw_circle(scx, scy, 9, ORANGE);
        lcd_draw_circle(scx, scy, 7, YELLOW);
        lcd_draw_circle(scx, scy, 4, WHITE);
        // Draw shrapnel lines (8 directions)
        for (int i = 0; i < 8; i++) {
            float angle = i * (M_PI / 4.0f);
            int x2 = scx + (int)(16 * cosf(angle));
            int y2 = scy + (int)(16 * sinf(angle));
            // Clamp line endpoints
            if (x2 < 0) x2 = 0;
            if (y2 < 0) y2 = 0;
            if (x2 > LCD_WIDTH-1) x2 = LCD_WIDTH-1;
            if (y2 > LCD_HEIGHT-1) y2 = LCD_HEIGHT-1;
            lcd_draw_line(scx, scy, x2, y2, (i % 2 == 0) ? ORANGE : YELLOW);
        }        // Play a rapid sequence of tones for explosion
        play_tone_conditional(400 + shake*100, 30);
        play_tone_conditional(200 + shake*80, 20);
        WaitUntil(30); // Short delay for shake frame
        // Clear explosion/shrapnel for this frame (clamp rectangle)
        int rx = scx-16, ry = scy-16, rw = 32, rh = 32;
        if (rx < 0) { rw += rx; rx = 0; }
        if (ry < 0) { rh += ry; ry = 0; }
        if (rx+rw > LCD_WIDTH) rw = LCD_WIDTH - rx;
        if (ry+rh > LCD_HEIGHT) rh = LCD_HEIGHT - ry;
        if (rw > 0 && rh > 0)
            lcd_fill_rect(rx, ry, rw, rh, GAME1942_BACKGROUND);
    }    // Final big flash
    lcd_draw_circle(cx, cy, 14, WHITE);
    play_tone_conditional(120, 60);
    WaitUntil(40);
    int rx = cx-16, ry = cy-16, rw = 32, rh = 32;
    if (rx < 0) { rw += rx; rx = 0; }
    if (ry < 0) { rh += ry; ry = 0; }
    if (rx+rw > LCD_WIDTH) rw = LCD_WIDTH - rx;
    if (ry+rh > LCD_HEIGHT) rh = LCD_HEIGHT - ry;
    if (rw > 0 && rh > 0)
        lcd_fill_rect(rx, ry, rw, rh, GAME1942_BACKGROUND);
}

// Short explosion effect for bullet-enemy collision
void short_explosion_effect(int x, int y) {
    // Clamp center to LCD bounds
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x > LCD_WIDTH-1) x = LCD_WIDTH-1;
    if (y > LCD_HEIGHT-1) y = LCD_HEIGHT-1;
    // Quick visual: small flash and circles    lcd_draw_circle(x, y, 6, YELLOW);
    lcd_draw_circle(x, y, 3, ORANGE);
    lcd_draw_point(x, y, WHITE);
    play_tone_conditional(700, 30); // Short, mid-high beep
    WaitUntil(30); // Very short delay
    // Clamp rectangle for clearing
    int rx = x-7, ry = y-7, rw = 14, rh = 14;
    if (rx < 0) { rw += rx; rx = 0; }
    if (ry < 0) { rh += ry; ry = 0; }
    if (rx+rw > LCD_WIDTH) rw = LCD_WIDTH - rx;
    if (ry+rh > LCD_HEIGHT) rh = LCD_HEIGHT - ry;
    if (rw > 0 && rh > 0)
        lcd_fill_rect(rx, ry, rw, rh, GAME1942_BACKGROUND);
}
