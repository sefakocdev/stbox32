#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "main.h"
#include "lcd_driver.h"
#include "joystick.h"

// Menu settings - Adjusted for horizontal/landscape orientation
#define MENU_TITLE_X 65
#define MENU_TITLE_Y LCD_WIDTH - 35
#define MENU_GAMES_START_Y LCD_WIDTH-70
#define MENU_GAME_SPACING 40
#define MENU_SELECTION_COLOR CYAN
#define MENU_TEXT_COLOR WHITE
#define MENU_TITLE_COLOR YELLOW
#define MENU_BACKGROUND_COLOR BLACK

// Sound icon settings
#define MENU_SOUND_ICON_X (LCD_WIDTH - MENU_SOUND_ICON_SIZE - 10)
#define MENU_SOUND_ICON_Y 10
#define MENU_SOUND_ICON_SIZE 24
#define MENU_SOUND_ICON_COLOR BLACK
#define MENU_SOUND_ICON_SELECTED_COLOR CYAN

// Game indices
typedef enum {
    GAME_BREAKOUT = 0,
    GAME_PONG = 1,
    GAME_1942 = 2,
    GAME_SNAKE = 3,
    GAME_TETRIS = 4,
    GAME_CARRACE = 5,
    GAME_COUNT = 6
} GameSelection;

// Add new selection for sound icon
#define MENU_SOUND_SELECTION 0
#define MENU_FIRST_GAME_SELECTION 1
#define MENU_TOTAL_SELECTIONS (GAME_COUNT + 1)

// Function declarations
void main_menu_init(void);
void main_menu_run(void);
void main_menu_draw(void);
void main_menu_update_selection(int direction);
void main_menu_draw_selection_box(void);
void main_menu_draw_title(void);
void main_menu_draw_games(void);
void main_menu_draw_sound_icon(bool selected, bool sound_on, uint16_t color);
void write_horizontal_text(int x, int y, const char* text, int font_size, uint16_t color);
void draw_horizontal_letter(int x, int y, char letter, uint16_t color, int font_size);
void draw_horizontal_text(int x, int y, const char* text, uint16_t color, int font_size);

#endif // MAIN_MENU_H
