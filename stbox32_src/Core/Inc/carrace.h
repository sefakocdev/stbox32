#ifndef CARRACE_H
#define CARRACE_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "main.h" // Include for HAL functions

// Game colors
#define CARRACE_BACKGROUND BLUE
#define PLAYER1_COLOR RED
#define PLAYER2_COLOR GREEN

#define CARRACE_FRAME_MS 10  // ~60 FPS (was 1ms = 1000 FPS!)

// Game state structure
typedef struct {
    int p1_score;
    int p2_score;
    int car1_x, car1_y;
    int car2_x, car2_y;
    int running;
    unsigned long start_time;
    unsigned long finish_time;
    int winner; // 0 = no winner, 1 = P1, 2 = P2
} CarRaceState;

#define BOX_SIZE 16
#define P1_START_X 22
#define P1_START_Y 5
#define P2_START_X 8
#define P2_START_Y 10
#define MOVE_STEP 8
#define FRONT_LIGHT_COLOR WHITE
#define TRACK_BORDER_COLOR YELLOW
#define FINISH_COLOR MAGENTA
#define TRACK_BORDER_THICKNESS 2

// Finish line coordinates
#define FINISH_LINE_X 10
#define FINISH_LINE_Y1 280
#define FINISH_LINE_Y2 310
#define FINISH_LINE_WIDTH 3

// Car collision detection
#define CAR_WIDTH 16
#define CAR_HEIGHT 16
#define MAX_TRACK_POINTS 20

// Track border points structure
typedef struct {
    int x1[MAX_TRACK_POINTS];
    int y1[MAX_TRACK_POINTS];
    int x2[MAX_TRACK_POINTS];
    int y2[MAX_TRACK_POINTS];
    int point_count;
} TrackBorders;

typedef enum {
    DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT, DIR_NONE
} CarDir;

// Function declarations
void carrace_play(void);
void carrace_init(void);
void carrace_draw_menu(void);
void carrace_wait_start(void);
void carrace_update(void);
void carrace_draw(int dir1, int dir2);
void draw_race_track(void);

// Traffic light functions
void lcd_fill_circle(uint16_t x_center, uint16_t y_center, uint16_t radius, uint16_t color);
void carrace_traffic_light_sequence(void);

// Collision detection functions
bool check_car_track_collision(int car_x, int car_y, CarDir car_dir);
bool line_rect_collision(int x1, int y1, int x2, int y2, int rect_x, int rect_y, int rect_w, int rect_h);
void init_track_borders(void);
void redraw_track_borders_in_area(int x, int y, int width, int height);

// Finish line and winning functions
bool check_car_finish_line(int car_x, int car_y, CarDir car_dir);
void carrace_show_winner_screen(void);
int carrace_wait_restart(void);

// Winner screen selection
#define WINNER_PLAY_SELECTION 0
#define WINNER_EXIT_SELECTION 1
#define WINNER_TOTAL_SELECTIONS 2

void carrace_draw_winner_symbols(int selected_option);
int carrace_handle_winner_input(void);
void carrace_play_winner_sound(void);

#endif // CARRACE_H
