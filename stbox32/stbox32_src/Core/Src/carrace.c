#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "carrace.h"
#include "lcd_driver.h"
#include "joystick.h"
#include "playmariosound.h"
#include "main.h"

extern TIM_HandleTypeDef htim3; // Ensure htim3 is configured for PWM elsewhere

static CarRaceState carrace_state;

static int prev_car1_x = P1_START_X, prev_car1_y = P1_START_Y;
static int prev_car2_x = P2_START_X, prev_car2_y = P2_START_Y;
static CarDir car1_dir = DIR_RIGHT;
static CarDir car2_dir = DIR_RIGHT;

// Track border storage for collision detection
static TrackBorders track_borders;

void carrace_init(void) {
    lcd_clear_screen(BLACK);
    carrace_state.p1_score = 0;
    carrace_state.p2_score = 0;
    carrace_state.car1_x = P1_START_X;
    carrace_state.car1_y = P1_START_Y;
    carrace_state.car2_x = P2_START_X;
    carrace_state.car2_y = P2_START_Y;
    carrace_state.running = 1;
    carrace_state.start_time = 0;
    carrace_state.finish_time = 0;
    carrace_state.winner = 0;
    
    // Initialize track borders for collision detection
    init_track_borders();
}

void carrace_draw_menu(void) {

	// Draw "STBOX32" title in the center top using custom horizontal letters with different colors
	const char* title = "CAR RACE";
	uint16_t colors[] = {RED, GREEN, MAGENTA, WHITE, YELLOW, CYAN, WHITE,BLUE};
	int letter_spacing = 30; // Space between letters
	  for (int i = 0; i < 8; i++) {
		draw_horizontal_letter(LCD_WIDTH - 50, 50  + (i * letter_spacing), title[i], colors[i], 16);
	}

	const char* player1_msg = "P1: RE0";
	draw_horizontal_text(LCD_WIDTH - 120, 75, player1_msg, PLAYER1_COLOR, 8);
	const char* player2_msg = "P2: GREEN";
	draw_horizontal_text(LCD_WIDTH - 120, 175, player2_msg, PLAYER2_COLOR, 8);

	const char* start_msg = "PRESS ANY BUTTON TO START";
	draw_horizontal_text(LCD_WIDTH - 160, 25, start_msg, CYAN, 8);
}

void carrace_wait_start(void) {
    unsigned start;
    start = HAL_GetTick();
    while (!Joystick_ReadButton(JOYSTICK_1) && !Joystick_ReadButton(JOYSTICK_2)) {
        // Use tick-based wait
        // Optionally, add a __WFI() or low-power wait here
    }
    // Small debounce (tick-based)
    while (HAL_GetTick() - start < 150) {}
}

void draw_car(int x, int y, uint16_t color, CarDir dir) {
    if (dir == DIR_UP) {
        // Body (width 12, height 8)
    	lcd_draw_rect(x + 2, y + 2, 12, 8, color);
        // Front lights (right)
        lcd_draw_rect(x + 14, y + 4, 2, 2, FRONT_LIGHT_COLOR);
        lcd_draw_rect(x + 14, y + 7, 2, 2, FRONT_LIGHT_COLOR);
        // Rear wheels at corners
        lcd_draw_rect(x, y, 3, 3, BLACK);
        lcd_draw_rect(x, y + 9, 3, 3, BLACK);
        // Front wheels (right)
        lcd_draw_rect(x + 11, y, 3, 3, BLACK);
        lcd_draw_rect(x + 11, y + 9, 3, 3, BLACK);
    } else if (dir == DIR_DOWN) {
        // Body (width 12, height 8)
    	lcd_draw_rect(x + 2, y + 2, 12, 8, color);
        // Front lights (left)
        lcd_draw_rect(x, y + 4, 2, 2, FRONT_LIGHT_COLOR);
        lcd_draw_rect(x, y + 7, 2, 2, FRONT_LIGHT_COLOR);
        // Rear wheels at corners
        lcd_draw_rect(x + 11, y, 3, 3, BLACK);
        lcd_draw_rect(x + 11, y + 9, 3, 3, BLACK);
        // Front wheels (left)
        lcd_draw_rect(x + 2, y, 3, 3, BLACK);
        lcd_draw_rect(x + 2, y + 9, 3, 3, BLACK);
    } else if (dir == DIR_LEFT) {
        // Body (width 8, height 12)
    	lcd_draw_rect(x + 2, y + 2, 8, 12, color);
        // Front lights (top)
        lcd_draw_rect(x + 4, y, 2, 2, FRONT_LIGHT_COLOR);
        lcd_draw_rect(x + 7, y, 2, 2, FRONT_LIGHT_COLOR);
        // Rear wheels at corners
        lcd_draw_rect(x, y + 11, 3, 3, BLACK);
        lcd_draw_rect(x + 9, y + 11, 3, 3, BLACK);
        // Front wheels (top)
        lcd_draw_rect(x, y + 2, 3, 3, BLACK);
        lcd_draw_rect(x + 9, y + 2, 3, 3, BLACK);
    } else if (dir == DIR_RIGHT) {
        // Body (width 8, height 12)
    	lcd_draw_rect(x + 2, y + 2, 8, 12, color);
        // Front lights (bottom)
        lcd_draw_rect(x + 4, y + 14, 2, 2, FRONT_LIGHT_COLOR);
        lcd_draw_rect(x + 7, y + 14, 2, 2, FRONT_LIGHT_COLOR);
        // Rear wheels at corners
        lcd_draw_rect(x, y, 3, 3, BLACK);
        lcd_draw_rect(x + 9, y, 3, 3, BLACK);
        // Front wheels (bottom)
        lcd_draw_rect(x, y + 11, 3, 3, BLACK);
        lcd_draw_rect(x + 9, y + 11, 3, 3, BLACK);
    }
}

void carrace_update(void) {
    int dir1, dir2;
    int car1_moved = 0, car2_moved = 0;
    
    // Read both joysticks simultaneously to avoid ADC conflicts
    Joystick_ReadBothDirections(&dir1, &dir2);
    // dir1 = JOYSTICK_1 direction (P2 - Green car)
    // dir2 = JOYSTICK_2 direction (P1 - Red car)
    
    // Move P2 (car2) with joystick 1 - with collision detection
    if (dir1 != JOY_NONE) {
        prev_car2_x = carrace_state.car2_x;
        prev_car2_y = carrace_state.car2_y;
        
        // Calculate new position based on direction
        int new_x = carrace_state.car2_x;
        int new_y = carrace_state.car2_y;
        CarDir new_dir = car2_dir;
        
        if (dir1 == JOY_LEFT) { new_y -= MOVE_STEP; new_dir = DIR_LEFT; }
        else if (dir1 == JOY_RIGHT) { new_y += MOVE_STEP; new_dir = DIR_RIGHT; }
        else if (dir1 == JOY_UP) { new_x += MOVE_STEP; new_dir = DIR_UP; }
        else if (dir1 == JOY_DOWN) { new_x -= MOVE_STEP; new_dir = DIR_DOWN; }
        else if (dir1 == JOY_UP_LEFT) { new_x += MOVE_STEP; new_y -= MOVE_STEP; new_dir = DIR_UP; }
        else if (dir1 == JOY_UP_RIGHT) { new_x += MOVE_STEP; new_y += MOVE_STEP; new_dir = DIR_UP; }
        else if (dir1 == JOY_DOWN_LEFT) { new_x -= MOVE_STEP; new_y -= MOVE_STEP; new_dir = DIR_DOWN; }
        else if (dir1 == JOY_DOWN_RIGHT) { new_x -= MOVE_STEP; new_y += MOVE_STEP; new_dir = DIR_DOWN; }
        
        // Screen boundary check
        if (new_x < 0) new_x = 0;
        if (new_y < 0) new_y = 0;
        if (new_x > LCD_WIDTH - BOX_SIZE) new_x = LCD_WIDTH - BOX_SIZE;
        if (new_y > LCD_HEIGHT - BOX_SIZE) new_y = LCD_HEIGHT - BOX_SIZE;
        
        // Check for track border collision - if collision detected, don't move (speed = 0)
        if (!check_car_track_collision(new_x, new_y, new_dir)) {
            carrace_state.car2_x = new_x;
            carrace_state.car2_y = new_y;
            car2_dir = new_dir;
            car2_moved = 1;
        }
        // If collision detected, car stays at previous position (speed = 0)
    }
    
    // Move P1 (car1) with joystick 2 - with collision detection
    if (dir2 != JOY_NONE) {
        prev_car1_x = carrace_state.car1_x;
        prev_car1_y = carrace_state.car1_y;
        
        // Calculate new position based on direction
        int new_x = carrace_state.car1_x;
        int new_y = carrace_state.car1_y;
        CarDir new_dir = car1_dir;
        
        if (dir2 == JOY_LEFT) { new_y -= MOVE_STEP; new_dir = DIR_LEFT; }
        else if (dir2 == JOY_RIGHT) { new_y += MOVE_STEP; new_dir = DIR_RIGHT; }
        else if (dir2 == JOY_UP) { new_x += MOVE_STEP; new_dir = DIR_UP; }
        else if (dir2 == JOY_DOWN) { new_x -= MOVE_STEP; new_dir = DIR_DOWN; }
        else if (dir2 == JOY_UP_LEFT) { new_x += MOVE_STEP; new_y -= MOVE_STEP; new_dir = DIR_UP; }
        else if (dir2 == JOY_UP_RIGHT) { new_x += MOVE_STEP; new_y += MOVE_STEP; new_dir = DIR_UP; }
        else if (dir2 == JOY_DOWN_LEFT) { new_x -= MOVE_STEP; new_y -= MOVE_STEP; new_dir = DIR_DOWN; }
        else if (dir2 == JOY_DOWN_RIGHT) { new_x -= MOVE_STEP; new_y += MOVE_STEP; new_dir = DIR_DOWN; }
        
        // Screen boundary check
        if (new_x < 0) new_x = 0;
        if (new_y < 0) new_y = 0;
        if (new_x > LCD_WIDTH - BOX_SIZE) new_x = LCD_WIDTH - BOX_SIZE;
        if (new_y > LCD_HEIGHT - BOX_SIZE) new_y = LCD_HEIGHT - BOX_SIZE;
        
        // Check for track border collision - if collision detected, don't move (speed = 0)
        if (!check_car_track_collision(new_x, new_y, new_dir)) {
            carrace_state.car1_x = new_x;
            carrace_state.car1_y = new_y;
            car1_dir = new_dir;
            car1_moved = 1;
        }
        // If collision detected, car stays at previous position (speed = 0)
    }
      // Redraw both cars whenever either car moves
    if (car1_moved || car2_moved) {
        // Clear car2's previous position if it moved
        if (car2_moved) {
            lcd_fill_rect(prev_car2_x, prev_car2_y, BOX_SIZE, BOX_SIZE, CARRACE_BACKGROUND);
            
            // Clear additional area for front lights that might extend beyond BOX_SIZE
            if (car2_dir == DIR_UP) {
                // Front lights extend to x+16, clear extra 2 pixels to the right
                lcd_fill_rect(prev_car2_x + BOX_SIZE, prev_car2_y + 4, 2, 6, CARRACE_BACKGROUND);
            } else if (car2_dir == DIR_RIGHT) {
                // Front lights extend to y+16, clear extra 2 pixels downward
                lcd_fill_rect(prev_car2_x + 4, prev_car2_y + BOX_SIZE, 6, 2, CARRACE_BACKGROUND);
            }
            
            // Redraw any track borders that might have been erased
            redraw_track_borders_in_area(prev_car2_x, prev_car2_y, BOX_SIZE + 2, BOX_SIZE + 2);
        }
        
        // Clear car1's previous position if it moved
        if (car1_moved) {
            lcd_fill_rect(prev_car1_x, prev_car1_y, BOX_SIZE, BOX_SIZE, CARRACE_BACKGROUND);
            
            // Clear additional area for front lights that might extend beyond BOX_SIZE
            if (car1_dir == DIR_UP) {
                // Front lights extend to x+16, clear extra 2 pixels to the right
                lcd_fill_rect(prev_car1_x + BOX_SIZE, prev_car1_y + 4, 2, 6, CARRACE_BACKGROUND);
            } else if (car1_dir == DIR_RIGHT) {
                // Front lights extend to y+16, clear extra 2 pixels downward
                lcd_fill_rect(prev_car1_x + 4, prev_car1_y + BOX_SIZE, 6, 2, CARRACE_BACKGROUND);
            }
            
            // Redraw any track borders that might have been erased
            redraw_track_borders_in_area(prev_car1_x, prev_car1_y, BOX_SIZE + 2, BOX_SIZE + 2);
        }
        
        // Always redraw both cars in their current positions
        draw_car(carrace_state.car2_x, carrace_state.car2_y, PLAYER2_COLOR, car2_dir);
        draw_car(carrace_state.car1_x, carrace_state.car1_y, PLAYER1_COLOR, car1_dir);
    }
    
    // Check for finish line crossing (only if at least one car moved)
    if ((car1_moved || car2_moved) && carrace_state.winner == 0) {
        if (check_car_finish_line(carrace_state.car1_x, carrace_state.car1_y, car1_dir)) {
            carrace_state.winner = 1; // P1 wins
            carrace_state.finish_time = HAL_GetTick();
            carrace_state.running = 0;
        } else if (check_car_finish_line(carrace_state.car2_x, carrace_state.car2_y, car2_dir)) {            carrace_state.winner = 2; // P2 wins
            carrace_state.finish_time = HAL_GetTick();
            carrace_state.running = 0;
        }
    }
}

// Draw a winding track with several turns
void draw_race_track(void) {
    // Draw only the blue path as a single thick yellow line (border)
    // LCD_WIDTH = 240, LCD_HEIGHT = 320
    int x1[] = {
        3, 3, 100, 100, 200, 200, 150, 40, 40, 170, 180, 210, 210, 10
    };
    int y1[] = {
        3, 70, 70, 40, 40, 80, 120, 120, 260, 200, 210, 195, 280, 280
    };

    int x2[] = {
        43, 40, 70, 70, 230, 230, 140, 70, 70, 170, 180, 235, 235, 10
    };
    int y2[] = {
        3, 40, 40, 15, 15, 100, 155, 155, 210, 170, 180, 140, 310, 310
    };


    int n = 14; // number of points

    for (int i = 0; i < n-1; i++) {
        lcd_draw_line(x1[i], y1[i], x1[i+1], y1[i+1], TRACK_BORDER_COLOR);
        for (int t = 1; t < TRACK_BORDER_THICKNESS; t++) {
            lcd_draw_line(x1[i]+t, y1[i], x1[i+1]+t, y1[i+1], TRACK_BORDER_COLOR);
            lcd_draw_line(x1[i]-t, y1[i], x1[i+1]-t, y1[i+1], TRACK_BORDER_COLOR);
        }
    }
    for (int i = 0; i < n-1; i++) {
        lcd_draw_line(x2[i], y2[i], x2[i+1], y2[i+1], TRACK_BORDER_COLOR);
        for (int t = 1; t < TRACK_BORDER_THICKNESS; t++) {
            lcd_draw_line(x2[i]+t, y2[i], x2[i+1]+t, y2[i+1], TRACK_BORDER_COLOR);
            lcd_draw_line(x2[i]-t, y2[i], x2[i+1]-t, y2[i+1], TRACK_BORDER_COLOR);
        }
    }

    int finish_x = 10;
    int finish_y1 = 280;
    int finish_y2 = 310;
	lcd_draw_line(finish_x, finish_y1, finish_x, finish_y2, FINISH_COLOR);
	lcd_draw_line(finish_x+1, finish_y1, finish_x+1, finish_y2, FINISH_COLOR);
	lcd_draw_line(finish_x-1, finish_y1, finish_x-1, finish_y2, FINISH_COLOR);


	lcd_fill_rect(50, 85, 50, 20, BLACK);	// body
	lcd_fill_rect(10, 93, 40, 4, BLACK);	// pole
	lcd_draw_circle(61, 95, 5, WHITE); 	// top circle
	lcd_draw_circle(75, 95, 5, WHITE);	// middle circle
	lcd_draw_circle(89, 95, 5, WHITE);	// bottom circle

}

// Initialize track border coordinates for collision detection
void init_track_borders(void) {
    // Copy the track coordinates from draw_race_track function
    int x1[] = {
        3, 3, 100, 100, 200, 200, 150, 40, 40, 170, 180, 210, 210, 10
    };
    int y1[] = {
        3, 70, 70, 40, 40, 80, 120, 120, 260, 200, 210, 195, 280, 280
    };

    int x2[] = {
        43, 40, 70, 70, 230, 230, 140, 70, 70, 170, 180, 235, 235, 10
    };
    int y2[] = {
        3, 40, 40, 15, 15, 100, 155, 155, 210, 170, 180, 140, 310, 310
    };

    track_borders.point_count = 14;
    
    for (int i = 0; i < track_borders.point_count; i++) {
        track_borders.x1[i] = x1[i];
        track_borders.y1[i] = y1[i];
        track_borders.x2[i] = x2[i];
        track_borders.y2[i] = y2[i];
    }
}

// Check if a line segment intersects with a rectangle (car bounds)
bool line_rect_collision(int x1, int y1, int x2, int y2, int rect_x, int rect_y, int rect_w, int rect_h) {
    // Check if line endpoints are inside rectangle
    if ((x1 >= rect_x && x1 <= rect_x + rect_w && y1 >= rect_y && y1 <= rect_y + rect_h) ||
        (x2 >= rect_x && x2 <= rect_x + rect_w && y2 >= rect_y && y2 <= rect_y + rect_h)) {
        return true;
    }
    
    // Check if line intersects any of the four rectangle edges    // Top edge
    if ((y1 <= rect_y && y2 >= rect_y) || (y1 >= rect_y && y2 <= rect_y)) {
        int x_intersect = x1 + (x2 - x1) * (rect_y - y1) / (y2 - y1);
        if (x_intersect >= rect_x && x_intersect <= rect_x + rect_w) {
            return true;
        }
    }
    
    // Bottom edge
    if ((y1 <= rect_y + rect_h && y2 >= rect_y + rect_h) || (y1 >= rect_y + rect_h && y2 <= rect_y + rect_h)) {
        int x_intersect = x1 + (x2 - x1) * (rect_y + rect_h - y1) / (y2 - y1);
        if (x_intersect >= rect_x && x_intersect <= rect_x + rect_w) {
            return true;
        }
    }
    
    // Left edge
    if ((x1 <= rect_x && x2 >= rect_x) || (x1 >= rect_x && x2 <= rect_x)) {
        int y_intersect = y1 + (y2 - y1) * (rect_x - x1) / (x2 - x1);
        if (y_intersect >= rect_y && y_intersect <= rect_y + rect_h) {
            return true;
        }
    }
    
    // Right edge
    if ((x1 <= rect_x + rect_w && x2 >= rect_x + rect_w) || (x1 >= rect_x + rect_w && x2 <= rect_x + rect_w)) {
        int y_intersect = y1 + (y2 - y1) * (rect_x + rect_w - x1) / (x2 - x1);
        if (y_intersect >= rect_y && y_intersect <= rect_y + rect_h) {
            return true;
        }
    }
    
    return false;
}

// Check if car collides with track borders
bool check_car_track_collision(int car_x, int car_y, CarDir car_dir) {
    int car_width, car_height;
    
    // Determine car dimensions based on direction
    if (car_dir == DIR_UP || car_dir == DIR_DOWN) {
        car_width = 16;  // includes wheels and lights
        car_height = 12;
    } else { // DIR_LEFT or DIR_RIGHT
        car_width = 12;
        car_height = 16;  // includes wheels and lights
    }
    
    // Check collision with all track border line segments
    for (int i = 0; i < track_borders.point_count - 1; i++) {
        // Check both border lines
        if (line_rect_collision(track_borders.x1[i], track_borders.y1[i], 
                               track_borders.x1[i+1], track_borders.y1[i+1],
                               car_x, car_y, car_width, car_height)) {
            return true;
        }
        
        if (line_rect_collision(track_borders.x2[i], track_borders.y2[i], 
                               track_borders.x2[i+1], track_borders.y2[i+1],
                               car_x, car_y, car_width, car_height)) {
            return true;
        }
        
        // Also check the thicker border lines (TRACK_BORDER_THICKNESS = 2)
        for (int t = 1; t < TRACK_BORDER_THICKNESS; t++) {
            // First border line with thickness
            if (line_rect_collision(track_borders.x1[i] + t, track_borders.y1[i], 
                                   track_borders.x1[i+1] + t, track_borders.y1[i+1],
                                   car_x, car_y, car_width, car_height)) {
                return true;
            }
            if (line_rect_collision(track_borders.x1[i] - t, track_borders.y1[i], 
                                   track_borders.x1[i+1] - t, track_borders.y1[i+1],
                                   car_x, car_y, car_width, car_height)) {
                return true;
            }
            
            // Second border line with thickness
            if (line_rect_collision(track_borders.x2[i] + t, track_borders.y2[i], 
                                   track_borders.x2[i+1] + t, track_borders.y2[i+1],
                                   car_x, car_y, car_width, car_height)) {
                return true;
            }
            if (line_rect_collision(track_borders.x2[i] - t, track_borders.y2[i], 
                                   track_borders.x2[i+1] - t, track_borders.y2[i+1],
                                   car_x, car_y, car_width, car_height)) {
                return true;
            }
        }
    }
    
    return false;
}

static unsigned long last_tick = 0;

// Additional note definitions for carrace winner sound
#define NOTE_C5  523
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_G5  784
#define NOTE_A5  880
#define NOTE_C6  1047

// Forward declaration of carrace_play_tone
static void carrace_play_tone(unsigned int freq, unsigned int duration_ms);

// Winner sound melody and durations
static int carrace_winner_melody[] = {
   NOTE_C5, NOTE_C5, NOTE_C5, NOTE_C5,
   NOTE_G5, NOTE_G5, NOTE_G5, NOTE_G5,
   NOTE_A5, NOTE_A5, NOTE_G5, NOTE_G5,
   NOTE_F5, NOTE_F5, NOTE_E5, NOTE_E5,
   NOTE_C5, NOTE_G5, NOTE_C6, NOTE_G5,
   NOTE_C6, NOTE_G5, NOTE_C6, REST
};

static int carrace_winner_note_durations[] = {
   8, 8, 8, 8,
   8, 8, 8, 8,
   8, 8, 8, 8,
   8, 8, 8, 8,
   4, 4, 4, 4,
   4, 4, 2, 2  // Total approximately 3 seconds
};

void carrace_play(void) {
    do {
        carrace_init();
        carrace_draw_menu();
        carrace_wait_start();
          // Set start time when race begins
        carrace_state.start_time = HAL_GetTick();
          lcd_clear_screen(CARRACE_BACKGROUND);
        draw_race_track();
        // Draw both cars at start
        draw_car(carrace_state.car1_x, carrace_state.car1_y, PLAYER1_COLOR, car1_dir);        draw_car(carrace_state.car2_x, carrace_state.car2_y, PLAYER2_COLOR, car2_dir);

        // Traffic light sequence before race starts
        carrace_traffic_light_sequence();
        
        // Reset start time after traffic light sequence
        carrace_state.start_time = HAL_GetTick();

        last_tick = HAL_GetTick();
        while (carrace_state.running) {
            unsigned long now = HAL_GetTick();
            if (now - last_tick >= CARRACE_FRAME_MS) {
                carrace_update();
                last_tick = now;
            }
        }
        
        // Show winner screen when race ends
        if (carrace_state.winner != 0) {
            carrace_show_winner_screen();
            int user_choice = carrace_wait_restart();
            
            if (user_choice == WINNER_EXIT_SELECTION) {
                // Exit to main menu
                return;
            }
            // If user_choice == WINNER_PLAY_SELECTION, continue loop to play again
        }
    } while (carrace_state.winner != 0); // Loop to allow restart
}

// Redraw track borders in a specific area to fix any erased borders
void redraw_track_borders_in_area(int x, int y, int width, int height) {
    // Check each track border line segment to see if it intersects with the given area
    for (int i = 0; i < track_borders.point_count - 1; i++) {
        // Check if line segment intersects with the clearing area
        bool line1_intersects = line_rect_collision(track_borders.x1[i], track_borders.y1[i], 
                                                   track_borders.x1[i+1], track_borders.y1[i+1],
                                                   x, y, width, height);
        bool line2_intersects = line_rect_collision(track_borders.x2[i], track_borders.y2[i], 
                                                   track_borders.x2[i+1], track_borders.y2[i+1],
                                                   x, y, width, height);
        
        // Redraw first border line if it intersects
        if (line1_intersects) {
            lcd_draw_line(track_borders.x1[i], track_borders.y1[i], 
                         track_borders.x1[i+1], track_borders.y1[i+1], TRACK_BORDER_COLOR);
            for (int t = 1; t < TRACK_BORDER_THICKNESS; t++) {
                lcd_draw_line(track_borders.x1[i]+t, track_borders.y1[i], 
                             track_borders.x1[i+1]+t, track_borders.y1[i+1], TRACK_BORDER_COLOR);
                lcd_draw_line(track_borders.x1[i]-t, track_borders.y1[i], 
                             track_borders.x1[i+1]-t, track_borders.y1[i+1], TRACK_BORDER_COLOR);
            }
        }
        
        // Redraw second border line if it intersects
        if (line2_intersects) {
            lcd_draw_line(track_borders.x2[i], track_borders.y2[i], 
                         track_borders.x2[i+1], track_borders.y2[i+1], TRACK_BORDER_COLOR);
            for (int t = 1; t < TRACK_BORDER_THICKNESS; t++) {
                lcd_draw_line(track_borders.x2[i]+t, track_borders.y2[i], 
                             track_borders.x2[i+1]+t, track_borders.y2[i+1], TRACK_BORDER_COLOR);
                lcd_draw_line(track_borders.x2[i]-t, track_borders.y2[i], 
                             track_borders.x2[i+1]-t, track_borders.y2[i+1], TRACK_BORDER_COLOR);
            }
        }    }
}

// Check if car crosses the finish line
bool check_car_finish_line(int car_x, int car_y, CarDir car_dir) {
    int car_width, car_height;
    
    // Determine car dimensions based on direction
    if (car_dir == DIR_UP || car_dir == DIR_DOWN) {
        car_width = 16;
        car_height = 12;
    } else { // DIR_LEFT or DIR_RIGHT
        car_width = 12;
        car_height = 16;
    }
    
    // Check if car overlaps with finish line area
    // Finish line is vertical at x=FINISH_LINE_X from y=FINISH_LINE_Y1 to y=FINISH_LINE_Y2
    if (car_x < FINISH_LINE_X + FINISH_LINE_WIDTH && 
        car_x + car_width > FINISH_LINE_X - FINISH_LINE_WIDTH &&
        car_y < FINISH_LINE_Y2 && 
        car_y + car_height > FINISH_LINE_Y1) {
        return true;
    }
    
    return false;
}

// Draw play and exit symbols for winner screen
void carrace_draw_winner_symbols(int selected_option) {
    int play_y = 200;
    int play_x = 30;
    int exit_y = 270;
    int exit_x = 30;
    int symbol_size = 24;
    
    // Draw Play symbol (triangle pointing right)
    uint16_t play_color = (selected_option == WINNER_PLAY_SELECTION) ? GREEN : WHITE;
    lcd_draw_line(play_x, play_y, play_x + symbol_size, play_y, play_color);
    lcd_draw_line(play_x, play_y, play_x + symbol_size/2, play_y + symbol_size, play_color);
    lcd_draw_line(play_x + symbol_size, play_y, play_x + symbol_size/2, play_y + symbol_size, play_color);
    
    // Draw Exit symbol (X)
    uint16_t exit_color = (selected_option == WINNER_EXIT_SELECTION) ? RED : WHITE;
    lcd_draw_line(exit_x, exit_y, exit_x + symbol_size, exit_y + symbol_size, exit_color);
    lcd_draw_line(exit_x + symbol_size, exit_y, exit_x, exit_y + symbol_size, exit_color);
    lcd_draw_line(exit_x + 1, exit_y, exit_x + symbol_size + 1, exit_y + symbol_size, exit_color);
    lcd_draw_line(exit_x + symbol_size + 1, exit_y, exit_x + 1, exit_y + symbol_size, exit_color);
}


// Handle input for winner screen and return selection (0=play again, 1=exit to menu)
int carrace_handle_winner_input(void) {
    int selection = WINNER_PLAY_SELECTION;
    
    carrace_draw_winner_symbols(selection);
//    carrace_draw_winner_selection_box(selection);
    
    while (1) {
        int joy1_dir = Joystick_ReadDirection(JOYSTICK_1);
        int joy2_dir = Joystick_ReadDirection(JOYSTICK_2);
        
        // Handle left/right selection
        if (joy1_dir == JOY_LEFT || joy2_dir == JOY_LEFT || 
            joy1_dir == JOY_RIGHT || joy2_dir == JOY_RIGHT) {
            selection = (selection == WINNER_PLAY_SELECTION) ? WINNER_EXIT_SELECTION : WINNER_PLAY_SELECTION;
            
            // Play selection sound
            if (is_sound_enabled()) {
                play_tone_conditional(NOTE_E7, 60); // Short beep
            }
            
            carrace_draw_winner_symbols(selection);
//            carrace_draw_winner_selection_box(selection);
            HAL_Delay(200); // Debounce
        }
          // Handle button press
        if (Joystick_ReadButton(JOYSTICK_1) || Joystick_ReadButton(JOYSTICK_2)) {
            // Play confirmation sound
            if (is_sound_enabled()) {
                play_tone_conditional(NOTE_C7, 100);
            }
            HAL_Delay(200); // Debounce
            return selection;
        }
        
        HAL_Delay(50);
    }
}

// Show winner screen with finish time and selection options
void carrace_show_winner_screen(void) {
    lcd_clear_screen(BLACK);
    
    // Calculate finish time in seconds
    unsigned long race_time_ms = carrace_state.finish_time - carrace_state.start_time;
    unsigned long seconds = race_time_ms / 1000;
    unsigned long milliseconds = race_time_ms % 1000;
    
    // Draw winner message
    const char* winner_msg;
    uint16_t winner_color;
    
    if (carrace_state.winner == 1) {
        winner_msg = "P1 WON";
        winner_color = PLAYER1_COLOR;
    } else if (carrace_state.winner == 2) {
        winner_msg = "P2 WON";
        winner_color = PLAYER2_COLOR;
    } else {
        winner_msg = "TIE";
        winner_color = WHITE;
    }
    
    // Display winner message
    draw_horizontal_text(LCD_WIDTH - 50, 110, winner_msg, winner_color, 16);
    
    // Display finish time
    char time_buffer[32];
    sprintf(time_buffer, "F1N1SH T1ME: %lu.%02lu SECS", seconds, milliseconds / 10);
    draw_horizontal_text(LCD_WIDTH - 120, 15, time_buffer, WHITE, 10);
    
    // Display instruction
    const char* instruction = "SELECT OPT1ON:";
    draw_horizontal_text(LCD_WIDTH - 160, 30, instruction, CYAN, 8);

    carrace_play_winner_sound();
}

// Wait for restart selection (returns 0=play again, 1=exit to menu)
int carrace_wait_restart(void) {
    return carrace_handle_winner_input();
}

void carrace_play_winner_sound(void)
{
    int length = sizeof(carrace_winner_melody) / sizeof(carrace_winner_melody[0]);
    for (int i = 0; i < length; i++) {
        int noteDuration = 1000 / carrace_winner_note_durations[i];
        carrace_play_tone(carrace_winner_melody[i], noteDuration);
    }
}

static void carrace_play_tone(unsigned int freq, unsigned int duration_ms)
{
    // Check if sound is enabled globally
    if (!is_sound_enabled()) {
        HAL_Delay(duration_ms); // Still wait for timing, but no sound
        return;
    }
    
    if (freq == 0) {
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

// Custom function to draw a filled circle (since LCD driver only has outline circle)
void lcd_fill_circle(uint16_t x_center, uint16_t y_center, uint16_t radius, uint16_t color) {
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x*x + y*y <= radius*radius) {
                lcd_draw_point(x_center + x, y_center + y, color);
            }
        }
    }
}

// Traffic light sequence: Red -> Yellow -> Green
void carrace_traffic_light_sequence(void) {
    // Traffic light coordinates (from draw_race_track) - Fixed order: Red=top, Yellow=middle, Green=bottom
    uint16_t red_light_x = 89, red_light_y = 95;      // Top light (red)
    uint16_t yellow_light_x = 75, yellow_light_y = 95; // Middle light (yellow)
    uint16_t green_light_x = 61, green_light_y = 95;   // Bottom light (green)
    uint16_t light_radius = 5;
    
    // Step 1: Red light for 1 second with low tone sound
    lcd_fill_circle(red_light_x, red_light_y, light_radius - 1, RED);
    carrace_play_tone(220, 200);  // Low A note for red light (warning tone)
    
    // Tick-based delay for 800ms (total 1 second with sound)
    unsigned long start_time = HAL_GetTick();
    while (HAL_GetTick() - start_time < 800) {
        // Wait using tick-based timing
    }
    
    // Step 2: Clear red, show yellow for 1 second with medium tone
    lcd_fill_circle(red_light_x, red_light_y, light_radius - 1, BLACK);
    lcd_fill_circle(yellow_light_x, yellow_light_y, light_radius - 1, YELLOW);
    carrace_play_tone(440, 200);  // Middle A note for yellow light (caution tone)
    
    // Tick-based delay for 800ms (total 1 second with sound)
    start_time = HAL_GetTick();
    while (HAL_GetTick() - start_time < 800) {
        // Wait using tick-based timing
    }
    
    // Step 3: Clear yellow, show green with high tone (race starts!)
    lcd_fill_circle(yellow_light_x, yellow_light_y, light_radius - 1, BLACK);
    lcd_fill_circle(green_light_x, green_light_y, light_radius - 1, GREEN);
    carrace_play_tone(880, 300);  // High A note for green light (go tone)
    // No additional delay - game starts immediately after green light sound
}
