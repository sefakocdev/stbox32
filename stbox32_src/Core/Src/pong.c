#include "pong.h"
#include "lcd_driver.h"
#include "joystick.h"
#include "playmariosound.h"  // For is_sound_enabled() function

extern TIM_HandleTypeDef htim3; // Ensure htim3 is configured for PWM elsewhere

#define BACKGROUND_COLOR BLACK
#define PADDLE_COLOR WHITE
#define PLAYER1_PADDLE_COLOR GREEN     // Bottom player (red)
#define PLAYER2_PADDLE_COLOR RED     // Top player (green)
#define BALL_COLOR CYAN
#define SCORE_COLOR YELLOW
#define CENTER_LINE_COLOR GRAY

static PongBall ball;
static PongPaddle left_paddle;
static PongPaddle right_paddle;
static PongGame game;

// Background music state variables
static int pong_music_enabled = 0;
static int pong_current_note = 0;
static unsigned int pong_note_start_time = 0;
static unsigned int pong_current_note_duration = 0;
static int pong_music_playing = 0;

// Pong background melody - upbeat and rhythmic
static int pong_melody[] = {
    NOTE_C5, NOTE_E5, NOTE_G5, NOTE_C5, 
    NOTE_D5, NOTE_F5, NOTE_A5, NOTE_D5,
    NOTE_E5, NOTE_G5, NOTE_B5, NOTE_E5,
    NOTE_C5, NOTE_E5, NOTE_G5, NOTE_C5,
    
    NOTE_A4, NOTE_C5, NOTE_E5, NOTE_A4,
    NOTE_B4, NOTE_D5, NOTE_F5, NOTE_B4,
    NOTE_C5, NOTE_E5, NOTE_G5, NOTE_C5,
    NOTE_G4, NOTE_B4, NOTE_D5, NOTE_G4,
    
    PONG_REST, NOTE_C5, PONG_REST, NOTE_E5,
    PONG_REST, NOTE_G5, PONG_REST, NOTE_C5,
    NOTE_F5, NOTE_E5, NOTE_D5, NOTE_C5,
    NOTE_G5, NOTE_F5, NOTE_E5, NOTE_D5,
    
    NOTE_C5, NOTE_G4, NOTE_E4, NOTE_C4,
    NOTE_D4, NOTE_A4, NOTE_F4, NOTE_D4,
    NOTE_E4, NOTE_B4, NOTE_G4, NOTE_E4,
    NOTE_C4, NOTE_G4, NOTE_E4, NOTE_C4
};

static int pong_note_durations[] = {
    8, 8, 8, 8,
    8, 8, 8, 8,
    8, 8, 8, 8,
    8, 8, 8, 8,
    
    8, 8, 8, 8,
    8, 8, 8, 8,
    8, 8, 8, 8,
    8, 8, 8, 8,
    
    4, 8, 4, 8,
    4, 8, 4, 8,
    8, 8, 8, 8,
    8, 8, 8, 8,
    
    4, 4, 4, 4,
    4, 4, 4, 4,
    4, 4, 4, 4,
    2, 2, 2, 2
};

static void pong_play_tone(unsigned int freq, unsigned int duration_ms)
{
    // Check if sound is enabled globally
    if (!is_sound_enabled()) {
        HAL_Delay(duration_ms); // Still wait for timing, but no sound
        return;
    }
    
    if (freq == PONG_REST) {
        HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_2);
        HAL_Delay(duration_ms);
        return;
    }

    unsigned int arr = 1000000 / freq - 1; // 1 MHz timer base frequency
    __HAL_TIM_SET_AUTORELOAD(&htim3, arr);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, arr / 2); // 50% duty

    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    HAL_Delay(duration_ms);
    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_2);
    HAL_Delay(10); // short delay between notes for background music
}

void playPongSound(void)
{
    int length = sizeof(pong_melody) / sizeof(pong_melody[0]);
    for (int i = 0; i < length; i++) {
        int noteDuration = 1000 / pong_note_durations[i]; // Convert to milliseconds
        pong_play_tone(pong_melody[i], noteDuration);
    }
}

// Non-blocking background music functions
void pongSound_start(void)
{
    // Only start background music if sound is enabled globally
    if (!is_sound_enabled()) {
        pong_music_enabled = 0;
        return;
    }
    
    pong_music_enabled = 1;
    pong_current_note = 0;
    pong_note_start_time = HAL_GetTick();
    pong_current_note_duration = 1000 / pong_note_durations[0];
    pong_music_playing = 0;
}

void pongSound_stop(void)
{
    pong_music_enabled = 0;
    pong_music_playing = 0;
    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_2);
}

void pongSound_update(void)
{
    // Check if sound is enabled globally - if not, stop any playing music
    if (!is_sound_enabled()) {
        if (pong_music_playing) {
            HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_2);
            pong_music_playing = 0;
        }
        pong_music_enabled = 0;
        return;
    }
    
    if (!pong_music_enabled) {
        return;
    }
    
    unsigned int current_time = HAL_GetTick();
    int melody_length = sizeof(pong_melody) / sizeof(pong_melody[0]);
    
    // Check if it's time to start or stop the current note
    if (current_time - pong_note_start_time >= pong_current_note_duration) {
        // Stop current note
        if (pong_music_playing) {
            HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_2);
            pong_music_playing = 0;
        }
        
        // Move to next note
        pong_current_note++;
        if (pong_current_note >= melody_length) {
            pong_current_note = 0; // Loop the melody
        }
        
        // Set up next note
        pong_note_start_time = current_time;
        pong_current_note_duration = 1000 / pong_note_durations[pong_current_note];
        
        // Start playing the new note if it's not a rest
        if (pong_melody[pong_current_note] != PONG_REST) {
            unsigned int freq = pong_melody[pong_current_note];
            unsigned int arr = 1000000 / freq - 1;
            __HAL_TIM_SET_AUTORELOAD(&htim3, arr);
            __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, arr / 2);
            HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
            pong_music_playing = 1;
        }
    }
}

void WaitUntil(unsigned int ms) {
    unsigned int start = HAL_GetTick();
    while ((HAL_GetTick() - start) < ms) {
        // Optional: insert __WFI() or small delay to reduce CPU usage
    }
}
static void draw_paddle(PongPaddle* paddle, uint16_t color) {
    lcd_fill_rect(paddle->x, paddle->y, PONG_PADDLE_WIDTH, PONG_PADDLE_HEIGHT, color);
}

static void clear_paddle(PongPaddle* paddle) {
    lcd_fill_rect(paddle->prev_x, paddle->y, PONG_PADDLE_WIDTH, PONG_PADDLE_HEIGHT, BACKGROUND_COLOR);
}

static void draw_ball(void) {
    lcd_fill_rect(ball.x, ball.y, PONG_BALL_SIZE, PONG_BALL_SIZE, BALL_COLOR);
}

static void clear_ball(void) {
    lcd_fill_rect(ball.prev_x, ball.prev_y, PONG_BALL_SIZE, PONG_BALL_SIZE, BACKGROUND_COLOR);
}

static void draw_center_line(void) {
    // Draw dotted center line (horizontal)
    for (int x = 0; x < LCD_WIDTH; x += 20) {
        lcd_fill_rect(x, LCD_HEIGHT/2 - 1, 10, 2, CENTER_LINE_COLOR);
    }
}

void pong_draw_score(void) {
    char score_str[20];
    
    // Clear score area
    lcd_fill_rect(80, LCD_HEIGHT/2 - 15, 80, 30, BACKGROUND_COLOR);
    
    // Draw top player score
    sprintf(score_str, "%d", game.left_score);
    lcd_display_string(90, LCD_HEIGHT/2 - 10, (const uint8_t*)score_str, 12, SCORE_COLOR);
    
    // Draw bottom player score  
    sprintf(score_str, "%d", game.right_score);
    lcd_display_string(130, LCD_HEIGHT/2 - 10, (const uint8_t*)score_str, 12, SCORE_COLOR);
    
    // Draw separator
    lcd_display_string(110, LCD_HEIGHT/2 - 10, (const uint8_t*)"-", 12, SCORE_COLOR);
}

void pong_reset_ball(void) {
    ball.x = LCD_WIDTH / 2 - PONG_BALL_SIZE / 2;
    ball.y = LCD_HEIGHT / 2 - PONG_BALL_SIZE / 2;
    ball.prev_x = ball.x;
    ball.prev_y = ball.y;
    
    // Random direction for ball
    ball.dx = (rand() % 2 == 0) ? PONG_BALL_SPEED_X : -PONG_BALL_SPEED_X;
    ball.dy = (rand() % 2 == 0) ? PONG_BALL_SPEED_Y : -PONG_BALL_SPEED_Y;
}

void pong_reset_paddles(void) {
    // Clear all paddle lines completely
    pong_clear_paddle_lines();
    
    // Reset paddle positions to center
    left_paddle.x = (LCD_WIDTH - PONG_PADDLE_WIDTH) / 2;
    left_paddle.prev_x = left_paddle.x;
    
    right_paddle.x = (LCD_WIDTH - PONG_PADDLE_WIDTH) / 2;
    right_paddle.prev_x = right_paddle.x;
    
    // Redraw paddles at center positions
    draw_paddle(&left_paddle, PLAYER2_PADDLE_COLOR);   // Top player (green)
    draw_paddle(&right_paddle, PLAYER1_PADDLE_COLOR);  // Bottom player (red)
}

void pong_clear_paddle_lines(void) {
    // Clear the entire top paddle line (where top paddle can move)
    lcd_fill_rect(0, PONG_TOP_PADDLE_Y, LCD_WIDTH, PONG_PADDLE_HEIGHT, BACKGROUND_COLOR);
    
    // Clear the entire bottom paddle line (where bottom paddle can move)
    lcd_fill_rect(0, PONG_BOTTOM_PADDLE_Y, LCD_WIDTH, PONG_PADDLE_HEIGHT, BACKGROUND_COLOR);
}

void pong_init(void) {
    // Set proper rotation for horizontal gameplay
    setRotation(1); // Ensure horizontal orientation for pong
    
    lcd_clear_screen(BACKGROUND_COLOR);
    
    // Initialize game state
    game.left_score = 0;
    game.right_score = 0;
    game.game_over = 0;
    game.winner = 0;// Initialize top paddle (controlled by joystick 1)
    left_paddle.x = (LCD_WIDTH - PONG_PADDLE_WIDTH) / 2;
    left_paddle.y = PONG_TOP_PADDLE_Y;
    left_paddle.prev_x = left_paddle.x;
    left_paddle.speed = PONG_PADDLE_SPEED;
    
    // Initialize bottom paddle (controlled by joystick 2)
    right_paddle.x = (LCD_WIDTH - PONG_PADDLE_WIDTH) / 2;
    right_paddle.y = PONG_BOTTOM_PADDLE_Y;
    right_paddle.prev_x = right_paddle.x;
    right_paddle.speed = PONG_PADDLE_SPEED;
    
    // Initialize ball
    pong_reset_ball();
      // Draw game elements
    draw_center_line();
    pong_draw_score();
    draw_paddle(&left_paddle, PLAYER2_PADDLE_COLOR);   // Top player (green)
    draw_paddle(&right_paddle, PLAYER1_PADDLE_COLOR);  // Bottom player (red)
    draw_ball();
}

void pong_play(void) {
    // Set proper rotation for horizontal display
    setRotation(1); // Ensure horizontal orientation

    lcd_clear_screen(BACKGROUND_COLOR);

    // Draw "STBOX32" title in the center top using custom horizontal letters with different colors
	const char* title = "PONG";
	uint16_t colors[] = {RED, GREEN, MAGENTA, YELLOW};
	int letter_spacing = 30; // Space between letters
	  for (int i = 0; i < 4; i++) {
		draw_horizontal_letter(LCD_WIDTH - 50, 120  + (i * letter_spacing), title[i], colors[i], 16);
	}

	const char* player1_msg = "P1: RE0";
	draw_horizontal_text(LCD_WIDTH - 120, 75, player1_msg, RED, 8);
	const char* player2_msg = "P2: GREEN";
	draw_horizontal_text(LCD_WIDTH - 120, 175, player2_msg, GREEN, 8);

	const char* start_msg = "PRESS ANY BUTTON TO START";
	draw_horizontal_text(LCD_WIDTH - 160, 25, start_msg, CYAN, 8);
      while (1) {
        if(Joystick_ReadButton(JOYSTICK_1) || Joystick_ReadButton(JOYSTICK_2))
            break;
    }

    pong_init();
    pongSound_start(); // Start background music when game begins

    while (!game.game_over) {
        pongSound_update(); // Update background music non-blockingly
        if (!pong_update()) {
            break;
        }
    }
    
    pongSound_stop(); // Stop background music when game ends
    pong_game_over();
    WaitUntil(3000);
}
int pong_update(void) {
	int delayCount = 2;

    // Read both joystick directions in a single ADC conversion
    int top_direction = JOY_NONE;
    int bottom_direction = JOY_NONE;
    Joystick_ReadBothDirections(&bottom_direction, &top_direction); // bottom = JOYSTICK_1, top = JOYSTICK_2    // === Update top paddle (controlled by JOYSTICK_2) ===
    left_paddle.prev_x = left_paddle.x;
    if (top_direction == JOY_DOWN && left_paddle.x > 0) {
        left_paddle.x -= left_paddle.speed;
        if (left_paddle.x < 0) left_paddle.x = 0;
    } else if (top_direction == JOY_UP && left_paddle.x + PONG_PADDLE_WIDTH < LCD_WIDTH) {
        left_paddle.x += left_paddle.speed;
        if (left_paddle.x + PONG_PADDLE_WIDTH > LCD_WIDTH)
            left_paddle.x = LCD_WIDTH - PONG_PADDLE_WIDTH;
    } else {
        top_direction = JOY_NONE;
    }    // === Update bottom paddle (controlled by JOYSTICK_1) ===
    right_paddle.prev_x = right_paddle.x;    if (bottom_direction == JOY_DOWN && right_paddle.x > 0) {
        right_paddle.x -= right_paddle.speed;
        if (right_paddle.x < 0) right_paddle.x = 0;
    } else if (bottom_direction == JOY_UP && right_paddle.x + PONG_PADDLE_WIDTH < LCD_WIDTH) {
        right_paddle.x += right_paddle.speed;
        if (right_paddle.x + PONG_PADDLE_WIDTH > LCD_WIDTH)
            right_paddle.x = LCD_WIDTH - PONG_PADDLE_WIDTH;
    } else {
        bottom_direction = JOY_NONE;
    }

    // === Redraw paddles if they moved ===
    if (top_direction != JOY_NONE) {
    	delayCount -= 1;
        clear_paddle(&left_paddle);
        draw_paddle(&left_paddle, PLAYER2_PADDLE_COLOR);  // Top player (green)
    }
    if (bottom_direction != JOY_NONE) {
    	delayCount -= 1;
        clear_paddle(&right_paddle);
        draw_paddle(&right_paddle, PLAYER1_PADDLE_COLOR); // Bottom player (red)
    }

    // === Update ball position ===
    ball.prev_x = ball.x;
    ball.prev_y = ball.y;
    ball.x += ball.dx;
    ball.y += ball.dy;

    // Ball collision with left/right screen edges
    if (ball.x <= 0 || ball.x + PONG_BALL_SIZE >= LCD_WIDTH) {
        ball.dx *= -1;
        if (ball.x <= 0) ball.x = 0;
        if (ball.x + PONG_BALL_SIZE >= LCD_WIDTH) ball.x = LCD_WIDTH - PONG_BALL_SIZE;
    }

    // === Ball collision with top paddle ===
    if (ball.y <= left_paddle.y + PONG_PADDLE_HEIGHT &&
        ball.y + PONG_BALL_SIZE >= left_paddle.y &&
        ball.x + PONG_BALL_SIZE >= left_paddle.x &&
        ball.x <= left_paddle.x + PONG_PADDLE_WIDTH &&
        ball.dy < 0) {
        ball.dy *= -1;
        ball.y = left_paddle.y + PONG_PADDLE_HEIGHT + 1;

        int paddle_center = left_paddle.x + PONG_PADDLE_WIDTH / 2;
        int ball_center = ball.x + PONG_BALL_SIZE / 2;
        int diff = ball_center - paddle_center;
        if (diff > 5) ball.dx = PONG_BALL_SPEED_X;
        else if (diff < -5) ball.dx = -PONG_BALL_SPEED_X;
    }

    // === Ball collision with bottom paddle ===
    if (ball.y + PONG_BALL_SIZE >= right_paddle.y &&
        ball.y <= right_paddle.y + PONG_PADDLE_HEIGHT &&
        ball.x + PONG_BALL_SIZE >= right_paddle.x &&
        ball.x <= right_paddle.x + PONG_PADDLE_WIDTH &&
        ball.dy > 0) {
        ball.dy *= -1;
        ball.y = right_paddle.y - PONG_BALL_SIZE - 1;

        int paddle_center = right_paddle.x + PONG_PADDLE_WIDTH / 2;
        int ball_center = ball.x + PONG_BALL_SIZE / 2;
        int diff = ball_center - paddle_center;
        if (diff > 5) ball.dx = PONG_BALL_SPEED_X;
        else if (diff < -5) ball.dx = -PONG_BALL_SPEED_X;
    }    // === Check for scoring ===
    if (ball.y < 0) {
        clear_ball();
        game.right_score++;
        pongSound_playScore(); // Play score sound effect
        pong_draw_score();
        if (game.right_score >= SCORE_TO_WIN) {
            game.game_over = 1;
            game.winner = 2;
            return 0;
        }
        pong_reset_paddles();
        WaitUntil(1000);
        pong_reset_ball();
    } else if (ball.y > LCD_HEIGHT) {
        clear_ball();
        game.left_score++;
        pongSound_playScore(); // Play score sound effect
        pong_draw_score();
        if (game.left_score >= SCORE_TO_WIN) {
            game.game_over = 1;
            game.winner = 1;
            return 0;
        }
        pong_reset_paddles();
        WaitUntil(1000);
        pong_reset_ball();
    }

    // === Redraw ball ===
    clear_ball();
    draw_ball();

    int delay = 100*(delayCount>0) + 40*(delayCount>1);
    WaitUntil(delay);

    return 1;
}

void pong_game_over(void) {
    lcd_clear_screen(BLACK);

    const char* winner_msg;
    uint16_t winner_color;
    
    if (game.winner == 1) {
    	winner_msg = "P1 WON";
    	winner_color = PLAYER2_PADDLE_COLOR;
    } else if (game.winner == 2) {
    	winner_msg = "P2 WON";
    	winner_color = PLAYER1_PADDLE_COLOR;
    }
    // Display winner message
    draw_horizontal_text(LCD_WIDTH - 50, 110, winner_msg, winner_color, 16);

    char final_score[30];
    sprintf(final_score, "F1NAL SCORE: %d - %d", game.left_score, game.right_score);
	draw_horizontal_text(LCD_WIDTH - 130, 35, final_score, YELLOW, 10);

    pongSound_playWinner();


}

void pong_draw(void) {
    draw_center_line();
    pong_draw_score();
    draw_paddle(&left_paddle, PLAYER2_PADDLE_COLOR);   // Top player (green)
    draw_paddle(&right_paddle, PLAYER1_PADDLE_COLOR);  // Bottom player (red)
    draw_ball();
}

// Score melody - quick celebratory sound (1 second) - starts high and energetic
static int score_melody[] = {
    NOTE_C6, NOTE_G5, NOTE_C6, NOTE_E5, NOTE_G5, NOTE_C6, NOTE_G5, NOTE_C6
};

static int score_note_durations[] = {
    6, 6, 6, 8, 8, 4, 8, 2  // Total approximately 1 second - faster and more exciting
};

// Winner melody - victory fanfare (3 seconds)
static int winner_melody[] = {
    NOTE_C5, NOTE_C5, NOTE_C5, NOTE_C5,
    NOTE_G5, NOTE_G5, NOTE_G5, NOTE_G5,
    NOTE_A5, NOTE_A5, NOTE_G5, NOTE_G5,
    NOTE_F5, NOTE_F5, NOTE_E5, NOTE_E5,
    NOTE_C5, NOTE_G5, NOTE_C6, NOTE_G5,
    NOTE_C6, NOTE_G5, NOTE_C6, REST
};

static int winner_note_durations[] = {
    8, 8, 8, 8,
    8, 8, 8, 8,
    8, 8, 8, 8,
    8, 8, 8, 8,
    4, 4, 4, 4,
    4, 4, 2, 2  // Total approximately 3 seconds
};

void pongSound_playScore(void)
{
    // Temporarily stop background music
    int was_playing = pong_music_enabled;
    pongSound_stop();
    
    int length = sizeof(score_melody) / sizeof(score_melody[0]);
    for (int i = 0; i < length; i++) {
        int noteDuration = 1000 / score_note_durations[i];
        pong_play_tone(score_melody[i], noteDuration);
    }
    
    // Resume background music if it was playing
    if (was_playing) {
        pongSound_start();
    }
}

void pongSound_playWinner(void)
{
    // Stop background music completely
    pongSound_stop();
    
    int length = sizeof(winner_melody) / sizeof(winner_melody[0]);
    for (int i = 0; i < length; i++) {
        int noteDuration = 1000 / winner_note_durations[i];
        pong_play_tone(winner_melody[i], noteDuration);
    }
}
