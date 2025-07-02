#include "breakout.h"

#include "lcd_driver.h"
#include "joystick.h"
#include "playmariosound.h" // For play_tone

#define BACKGROUND_COLOR BLACK
#define PADDLE_COLOR CYAN
#define BALL_COLOR WHITE

int isInit = 0;
int activeBrickCount = BRICK_ROWS*BRICK_COLS;

const uint16_t brick_colors[5] = {
		RED,
		BLUE,
		YELLOW,
		MAGENTA,
		GREEN
};

static Ball ball;
static Paddle paddle;
static Brick bricks[BRICK_ROWS * BRICK_COLS];

static void draw_paddle(void) {
    lcd_fill_rect(paddle.x, PADDLE_Y, PADDLE_WIDTH, PADDLE_HEIGHT, PADDLE_COLOR);
}

static void clear_paddle(void) {
    lcd_fill_rect(paddle.prev_paddle_x, PADDLE_Y, PADDLE_WIDTH, PADDLE_HEIGHT, BACKGROUND_COLOR);
}

static void draw_ball(void) {
    lcd_fill_rect(ball.x, ball.y, BALL_SIZE, BALL_SIZE, BALL_COLOR);
}

static void clear_ball(void) {
    lcd_fill_rect(ball.x, ball.y, BALL_SIZE, BALL_SIZE, BACKGROUND_COLOR);
}

static void draw_bricks(void) {
    for (int i = 0; i < BRICK_ROWS * BRICK_COLS; i++) {
        if (bricks[i].active) {
            lcd_fill_rect(bricks[i].x, bricks[i].y, BRICK_WIDTH - 2, BRICK_HEIGHT - 2, bricks[i].color);
        }
    }
}

// Sound effect functions for breakout
void play_brick_sound(void) {
    play_tone_conditional(600, 30); // Brick: mid beep
}
void play_edge_sound(void) {
    play_tone_conditional(300, 20); // Edge: low short beep
}
void play_paddle_sound(void)
{
    // New paddle sound: double quick beep
    play_tone_conditional(1000, 20); // Higher pitch
    play_tone_conditional(600, 15);  // Quick lower echo
}

// Win and game over sound effects for breakout
void breakout_win_sound(void) {
    // Rising melody for win (3 seconds)
    for (int i = 0; i < 6; i++) {
        play_tone_conditional(600 + i*120, 400); // 600Hz to 1200Hz, 400ms each
    }
}
void breakout_gameover_sound(void) {
    // Descending/falling melody for game over (3 seconds)
    for (int i = 0; i < 6; i++) {
        play_tone_conditional(1200 - i*180, 400); // 1200Hz to 300Hz, 400ms each
    }
}

void breakout_init(int pSpeed) {
    // Set proper rotation for horizontal gameplay
    setRotation(1); // Ensure horizontal orientation for breakout
    
    lcd_clear_screen(BACKGROUND_COLOR);

    paddle.x = (LCD_WIDTH - PADDLE_WIDTH) / 2;
    paddle.prev_paddle_x = (LCD_WIDTH - PADDLE_WIDTH) / 2;
    paddle.speed =  3*pSpeed;

    ball.x = LCD_WIDTH / 2;
    ball.y = LCD_HEIGHT / 2;
    ball.dx = pSpeed;
    ball.dy = pSpeed;

    for (int row = 0; row < BRICK_ROWS; row++) {
        for (int col = 0; col < BRICK_COLS; col++) {
            int index = row * BRICK_COLS + col;
            bricks[index].x = col * BRICK_WIDTH + 1;
            bricks[index].y = row * BRICK_HEIGHT + 1;
            bricks[index].active = true;
            bricks[index].color = brick_colors[row];
        }
    }

    draw_bricks();
    draw_paddle();
    draw_ball();
}

void breakout_play(int pSpeed) {
    // Set proper rotation for horizontal display
    setRotation(1); // Ensure horizontal orientation
    
    lcd_clear_screen(BACKGROUND_COLOR);
	lcd_display_string(80, 100, (const uint8_t *)"B",  12, BLUE);
	lcd_display_string(90, 100, (const uint8_t *)"R",  12, MAGENTA);
	lcd_display_string(100, 100, (const uint8_t *)"E",  12, GREEN);
	lcd_display_string(110, 100, (const uint8_t *)"A",  12, WHITE);
	lcd_display_string(120, 100, (const uint8_t *)"K",  12, RED);
	lcd_display_string(130, 100, (const uint8_t *)"O",  12, CYAN);
	lcd_display_string(140, 100, (const uint8_t *)"U",  12, YELLOW);
	lcd_display_string(150, 100, (const uint8_t *)"T",  12, GREEN);
	lcd_display_string(160, 100, (const uint8_t *)"!",  12, BLUE);

	lcd_display_string(60, 180, (const uint8_t *)"Press Button To Start",  12, CYAN);
	while (1) {
		if(Joystick_ReadButton(JOYSTICK_1))
			break;
		HAL_Delay(50);
	}

	breakout_init(pSpeed);
	int isCont=1;

	while (1)
	{
	  if(isCont){
		  isCont = breakout_update();
	  }
	  else break;

	}

	HAL_Delay(1000);
}

int breakout_update() {
	int direction = Joystick_ReadDirection(JOYSTICK_1);
	paddle.prev_paddle_x = paddle.x;

	// Paddle movement according to joystick direction with boundary control
	// Handle both pure LEFT and diagonal LEFT movements
	if ((direction == JOY_DOWN || direction == JOY_DOWN_RIGHT || direction == JOY_DOWN_LEFT) && paddle.x > 0) {
	    paddle.x -= paddle.speed;
	    if (paddle.x < 0) paddle.x = 0;
	} 
	// Handle both pure RIGHT and diagonal RIGHT movements  
	else if ((direction == JOY_UP || direction == JOY_UP_RIGHT || direction == JOY_UP_LEFT) && paddle.x + PADDLE_WIDTH < LCD_WIDTH) {
	    paddle.x += paddle.speed;
	    if (paddle.x + PADDLE_WIDTH > LCD_WIDTH)
	        paddle.x = LCD_WIDTH - PADDLE_WIDTH;
	} else {
	    direction = JOY_NONE; // Don't move if it would go off screen
	}

    // Update if position has changed
	if (direction != JOY_NONE) {
		clear_paddle();
		draw_paddle();
	}

    // Ball movement
    clear_ball();
    ball.x += ball.dx;
    ball.y += ball.dy;

    // Wall collision
    if (ball.x <= 0 || ball.x + BALL_SIZE >= LCD_WIDTH) {
        ball.dx *= -1;
        play_edge_sound();
    }
    if (ball.y <= 0) {
        ball.dy *= -1;
        play_edge_sound();
    }

    // Paddle collision
    if (ball.y + BALL_SIZE >= PADDLE_Y &&
        ball.x + BALL_SIZE >= paddle.x &&
        ball.x <= paddle.x + PADDLE_WIDTH) {
        ball.dy *= -1;
        ball.y = PADDLE_Y - BALL_SIZE - 1;
        play_paddle_sound();
    }

    // Brick collision
    for (int i = 0; i < BRICK_ROWS * BRICK_COLS; i++) {
        if (!bricks[i].active) continue;

        if (ball.x + BALL_SIZE >= bricks[i].x &&
            ball.x <= bricks[i].x + BRICK_WIDTH &&
            ball.y + BALL_SIZE >= bricks[i].y &&
            ball.y <= bricks[i].y + BRICK_HEIGHT) {
            bricks[i].active = false;
            lcd_fill_rect(bricks[i].x, bricks[i].y, BRICK_WIDTH - 2, BRICK_HEIGHT - 2, BACKGROUND_COLOR);
            ball.dy *= -1;
            activeBrickCount -= 1;
            play_brick_sound();
            // win
			if (activeBrickCount == 0) {
				breakout_won();
				return 0;
			}
            break;
        }
    }

    // Reset game if ball falls down
    if (ball.y > LCD_HEIGHT) {
    	breakout_game_over();
        return 0;
    }

    draw_ball();

    if (direction == JOY_NONE) {
  	  HAL_Delay(40);
	}
    return 1;
}

void breakout_won(void) {
    lcd_clear_screen(BLACK);
    lcd_display_string(100, 150, (uint8_t*)"YOU WON!", 12, GREEN);
    breakout_win_sound();
    HAL_Delay(1000);
}

void breakout_game_over(void) {
    lcd_clear_screen(BLACK);
    lcd_display_string(100, 150, (uint8_t*)"GAME OVER!", 12, RED);
    breakout_gameover_sound();
    HAL_Delay(3000);
}

void breakout_draw(void) {
    draw_bricks();
    draw_paddle();
    draw_ball();
}

