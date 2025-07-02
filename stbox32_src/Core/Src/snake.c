#include "snake.h"
#include "lcd_driver.h"
#include "joystick.h"
#include "main.h"
#include "playmariosound.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Game state variables
static Snake snake;
static Apple apple;
static bool game_running = false;

void snake_init(void)
{
    // Initialize random seed
    srand(HAL_GetTick());
    
    // Clear screen with dark green background
    lcd_clear_screen(SNAKE_BACKGROUND_COLOR);
    
    // Draw border
    snake_draw_border();
    
    // Reset game state
    snake_reset_game();
}

void snake_reset_game(void)
{
    // Initialize snake in the center
    snake.length = SNAKE_INITIAL_LENGTH;
    snake.direction = SNAKE_DIR_RIGHT;
    snake.next_direction = SNAKE_DIR_RIGHT;
    snake.game_over = false;
    snake.score = 0;
    
    // Place snake in center of screen
    int start_x = SNAKE_GRID_WIDTH / 2;
    int start_y = SNAKE_GRID_HEIGHT / 2;
    
    for (int i = 0; i < snake.length; i++) {
        snake.body[i].x = start_x - i;
        snake.body[i].y = start_y;
    }
    
    // Spawn first apple
    snake_spawn_apple();
    
    // Clear screen and redraw everything
    lcd_clear_screen(SNAKE_BACKGROUND_COLOR);
    snake_draw_border();
    snake_draw_game();
}

void snake_draw_border(void)
{
    // Draw thick white border around the game area (5 pixels)
    for (int i = 0; i < 5; i++) {
        lcd_draw_rect(i, i, LCD_WIDTH - (2 * i), LCD_HEIGHT - (2 * i), SNAKE_BORDER_COLOR);
    }
}

void snake_draw_cell(int x, int y, uint16_t color)
{
    if (x >= 0 && x < SNAKE_GRID_WIDTH && y >= 0 && y < SNAKE_GRID_HEIGHT) {
        int pixel_x = x * SNAKE_CELL_SIZE + 5; // +5 for thick border
        int pixel_y = y * SNAKE_CELL_SIZE + 5; // +5 for thick border
        lcd_fill_rect(pixel_x, pixel_y, SNAKE_CELL_SIZE - 1, SNAKE_CELL_SIZE - 1, color);
    }
}

void snake_draw_apple(void)
{
    if (!apple.exists) return;

    int base_x = apple.x * SNAKE_CELL_SIZE + 5;
    int base_y = apple.y * SNAKE_CELL_SIZE + 5;

    // Shrink size slightly
    int radius_x = SNAKE_CELL_SIZE / 2 - 1; // slightly smaller width
    int radius_y = SNAKE_CELL_SIZE / 2 - 1; // slightly taller than wide

    // Apple body (slightly oval and asymmetric)
    for (int dy = 0; dy < SNAKE_CELL_SIZE; dy++) {
        for (int dx = 0; dx < SNAKE_CELL_SIZE; dx++) {
            float nx = (dx - SNAKE_CELL_SIZE / 2.0f) / radius_x;
            float ny = (dy - SNAKE_CELL_SIZE / 2.0f) / radius_y;

            // Slight asymmetry added to make it less "perfect"
            if (nx * nx + ny * ny <= 1.0f &&
                (dx != 0 && dx != SNAKE_CELL_SIZE - 1) &&
                (dy != 0 && dy != SNAKE_CELL_SIZE - 1)) {
                lcd_draw_point(base_x + dx, base_y + dy, SNAKE_APPLE_COLOR);
            }
        }
    }

    // Apple stem (2 vertical pixels at top middle)
    int stem_x = base_x + SNAKE_CELL_SIZE / 2;
    int stem_y = base_y + 1;
    lcd_draw_point(stem_x, stem_y, BROWN);
    lcd_draw_point(stem_x, stem_y + 1, BROWN);
}

void snake_draw_game(void)
{
    // Draw snake body
    for (int i = 0; i < snake.length; i++) {
        snake_draw_cell(snake.body[i].x, snake.body[i].y, SNAKE_BODY_COLOR);
    }
    
    // Draw apple
    snake_draw_apple();
}

void snake_spawn_apple(void)
{
    bool valid_position = false;
    int attempts = 0;
    
    while (!valid_position && attempts < 100) {
        // Generate random position within game bounds (excluding border)
        apple.x = 1 + (rand() % (SNAKE_GRID_WIDTH - 2));
        apple.y = 1 + (rand() % (SNAKE_GRID_HEIGHT - 2));
        
        // Check if apple position conflicts with snake body
        valid_position = true;
        for (int i = 0; i < snake.length; i++) {
            if (snake.body[i].x == apple.x && snake.body[i].y == apple.y) {
                valid_position = false;
                break;
            }
        }
        attempts++;
    }
    
    apple.exists = true;
}

bool snake_check_collision(void)
{
    // Check wall collision (including border)
    if (snake.body[0].x <= 0 || snake.body[0].x >= SNAKE_GRID_WIDTH - 1 ||
        snake.body[0].y <= 0 || snake.body[0].y >= SNAKE_GRID_HEIGHT - 1) {
        return true;
    }
    
    // Check self collision
    for (int i = 1; i < snake.length; i++) {
        if (snake.body[0].x == snake.body[i].x && snake.body[0].y == snake.body[i].y) {
            return true;
        }
    }
    
    return false;
}

bool snake_check_apple_collision(void)
{
    if (apple.exists && snake.body[0].x == apple.x && snake.body[0].y == apple.y) {
        return true;
    }
    return false;
}

void snake_handle_input(void)
{
    // Read both joysticks individually
    int joystick1_direction = Joystick_ReadDirection(JOYSTICK_1);
    int joystick2_direction = Joystick_ReadDirection(JOYSTICK_2);
    
    // Use either joystick input (prioritize joystick1 if both are active)
    int input_direction = (joystick1_direction != JOY_NONE) ? joystick1_direction : joystick2_direction;
    
    // Convert joystick input to snake direction, but prevent reverse direction
    switch (input_direction) {
        case JOY_LEFT:
            if (snake.direction != SNAKE_DIR_DOWN) {
                snake.next_direction = SNAKE_DIR_UP;
            }
            break;
        case JOY_RIGHT:
            if (snake.direction != SNAKE_DIR_UP) {
                snake.next_direction = SNAKE_DIR_DOWN;
            }
            break;
        case JOY_DOWN:
            if (snake.direction != SNAKE_DIR_RIGHT) {
                snake.next_direction = SNAKE_DIR_LEFT;
            }
            break;
        case JOY_UP:
            if (snake.direction != SNAKE_DIR_LEFT) {
                snake.next_direction = SNAKE_DIR_RIGHT;
            }
            break;
    }
}

void snake_move(void)
{
    // Update direction
    snake.direction = snake.next_direction;
    
    // Clear tail cell before moving
    int tail_x = snake.body[snake.length - 1].x;
    int tail_y = snake.body[snake.length - 1].y;
    snake_draw_cell(tail_x, tail_y, SNAKE_BACKGROUND_COLOR);
    
    // Move body segments (from tail to head)
    for (int i = snake.length - 1; i > 0; i--) {
        snake.body[i] = snake.body[i - 1];
    }
    
    // Move head based on direction
    switch (snake.direction) {
        case SNAKE_DIR_UP:
            snake.body[0].y--;
            break;
        case SNAKE_DIR_DOWN:
            snake.body[0].y++;
            break;
        case SNAKE_DIR_LEFT:
            snake.body[0].x--;
            break;
        case SNAKE_DIR_RIGHT:
            snake.body[0].x++;
            break;
    }    // Check apple collision
    if (snake_check_apple_collision()) {
        // Play quick growing sound effect (single ascending note)
        play_tone_conditional(NOTE_C7, 60);
        
        // Clear the apple area completely before growing snake
        snake_draw_cell(apple.x, apple.y, SNAKE_BACKGROUND_COLOR);
        
        // Grow snake - restore the tail that was just removed
        snake.body[snake.length].x = tail_x;
        snake.body[snake.length].y = tail_y;
        snake.length++;
        snake.score++;
        
        // Redraw the tail cell that we cleared
        snake_draw_cell(tail_x, tail_y, SNAKE_BODY_COLOR);
        
        // Remove apple and spawn new one
        apple.exists = false;
        snake_spawn_apple();
        snake_draw_apple();
    }
      // Check collisions
    if (snake_check_collision()) {
        // Play quick hit sound effect (single low note)
        play_tone_conditional(NOTE_E6, 100);
        
        snake.game_over = true;
        return;
    }
    
    // Draw new head
    snake_draw_cell(snake.body[0].x, snake.body[0].y, SNAKE_BODY_COLOR);
}

void snake_update(void)
{
    if (!snake.game_over) {
        snake_handle_input();
        snake_move();
    }
}

void snake_show_game_over(void)
{

    // Clear a section in the middle for game over text
    lcd_clear_screen(BLACK);

    // Display "GAME OVER" using horizontal text
    const char* game_over_text = "GAME OVER";
    draw_horizontal_text(180, 80, game_over_text, SNAKE_GAME_OVER_COLOR, 14);

    // Display final score using horizontal text
    char score_text[20];
    snprintf(score_text, sizeof(score_text), "F1NAL SCORE: %d", snake.score);
    draw_horizontal_text(110, 80, score_text, MAGENTA, 10);


    // Play game over melody first
    static bool melody_played = false;
    if (!melody_played) {
        // Game over melody (sad descending notes)
    	for(int i=0; i<3; i++) {
			play_tone_conditional(NOTE_C7, 300);
			HAL_Delay(50);
			play_tone_conditional(NOTE_AS6, 300);
			HAL_Delay(50);
			play_tone_conditional(NOTE_A6, 300);
			HAL_Delay(50);
			play_tone_conditional(NOTE_G6, 300);
			HAL_Delay(50);
			play_tone_conditional(NOTE_F7, 600);
			HAL_Delay(100);
			play_tone_conditional(NOTE_E6, 800);
			HAL_Delay(50);
    	}
        melody_played = true;
    }
    
    // Reset melody flag for next game over
    if (!snake.game_over) {
        melody_played = false;
    }
}

void snake_show_menu(void)
{
    // Clear screen with dark green background
    lcd_clear_screen(SNAKE_BACKGROUND_COLOR);
    
    // Draw decorative border
    lcd_draw_rect(5, 5, LCD_WIDTH - 10, LCD_HEIGHT - 10, SNAKE_BORDER_COLOR);
    lcd_draw_rect(6, 6, LCD_WIDTH - 12, LCD_HEIGHT - 12, SNAKE_BORDER_COLOR);
    
    // Draw "SNAKE" title at the top
    const char* title = "SNAKE";
    int title_start_x = 60;
    int title_start_y = 30;
    uint16_t title_colors[] = {GREEN, GREEN, GREEN, GREEN, GREEN}; // All green for snake theme
    
    for (int i = 0; i < 5; i++) {
        draw_horizontal_letter(title_start_x, title_start_y + (i * 25), title[i], title_colors[i], 18);
    }
    
    // Draw instructions
    const char* instruction1 = "SNAKE";
    draw_horizontal_text(150, 120, instruction1, BLUE, 16);
    
    
    // Draw "PRESS START" message
    const char* start_msg = "PRESS BUTTON TO START";
    draw_horizontal_text(100, 55, start_msg, BLACK, 8);
}

bool snake_wait_for_start(void)
{
    while (true) {
        // Check for button press from either joystick
        if (Joystick_ReadButton(JOYSTICK_1) || Joystick_ReadButton(JOYSTICK_2)) {
            HAL_Delay(200); // Debounce delay
            return true;
        }
        
        // Check for exit (both buttons pressed simultaneously)
        static int button_hold_count = 0;
        if (Joystick_ReadButton(JOYSTICK_1) && Joystick_ReadButton(JOYSTICK_2)) {
            button_hold_count++;
            if (button_hold_count > 20) { // Hold for about 1 second
                return false; // Exit to main menu
            }
        } else {
            button_hold_count = 0;
        }
        
        HAL_Delay(50);
    }
}

void snake_play(void)
{
    // Show menu first
    snake_show_menu();
    
    // Wait for player to press start
    if (!snake_wait_for_start()) {
        return; // Exit if player chose to go back to main menu
    }
    
    // Initialize and start the game
    snake_init();
    game_running = true;
      while (game_running) {
        if (!snake.game_over) {
            snake_update();
            HAL_Delay(150); // Game speed - adjust as needed
        } else {
            static uint32_t game_over_start_time = 0;
            static bool game_over_displayed = false;
            
            // Record the time when game over first occurs
            if (!game_over_displayed) {
                game_over_start_time = HAL_GetTick();
                game_over_displayed = true;
                snake_show_game_over();
            }
            
            // Check if 3 seconds have passed
            if (HAL_GetTick() - game_over_start_time >= 3000) {
                // Return to main menu after 3 seconds
                game_running = false;
                game_over_displayed = false; // Reset for next game
                return;
            }
            
            // Allow manual restart during the 3-second period
            if (Joystick_ReadButton(JOYSTICK_1) || Joystick_ReadButton(JOYSTICK_2)) {
                snake_reset_game();
                game_over_displayed = false; // Reset flag
                HAL_Delay(200); // Debounce delay
            }
        }
        
        // Check for exit (both buttons pressed simultaneously might exit to menu)
        static int button_hold_count = 0;
        if (Joystick_ReadButton(JOYSTICK_1) && Joystick_ReadButton(JOYSTICK_2)) {
            button_hold_count++;
            if (button_hold_count > 20) { // Hold for about 1 second
                game_running = false;
            }
        } else {
            button_hold_count = 0;
        }
        
        HAL_Delay(50); // Main loop delay
    }
}
