#include "main_menu.h"
#include "pong.h"
#include "breakout.h"
#include "playmariosound.h"
#include "game1942.h"
#include "snake.h"
#include "tetris.h"
#include "carrace.h"

static GameSelection current_selection = GAME_BREAKOUT;
static int menu_selection = MENU_FIRST_GAME_SELECTION; // Initial selection is first game
static bool menu_active = true;
static bool sound_on = true;

void main_menu_init(void)
{
    current_selection = GAME_BREAKOUT;
    menu_selection = MENU_FIRST_GAME_SELECTION;
    menu_active = true;
    lcd_clear_screen(MENU_BACKGROUND_COLOR);
    main_menu_draw();
}

void main_menu_draw_title(void)
{
    // Draw "STBOX32" title in the center top using custom horizontal letters with different colors
    const char* title = "STBOX32";
    uint16_t colors[] = {RED, GREEN, BLUE, YELLOW, CYAN, MAGENTA, WHITE}; // Different color for each letter
    
    int letter_spacing = 30; // Space between letters
    int title_start_y = MENU_TITLE_X;
    int title_x = MENU_TITLE_Y;
      for (int i = 0; i < 7; i++) {
        draw_horizontal_letter(title_x, title_start_y  + (i * letter_spacing), title[i], colors[i], 16);
    }
}

void main_menu_draw_games(void)
{
    // Draw game options using custom letter drawing to match title orientation
    const char* game1 = "1: BREAKOUT";
    const char* game2 = "2: PONG";
    const char* game3 = "3: 1942";
    const char* game4 = "4: SNAKE";
    const char* game5 = "5: TETR1S";
    const char* game6 = "6: CAR RACE";
    
    // Position games below the title line, centered horizontally
    int game1_x = MENU_TITLE_Y - 40;
    int game2_x = game1_x - 30;
    int game3_x = game2_x - 30;
    int game4_x = game3_x - 30;
    int game5_x = game4_x - 30;
    int game6_x = game5_x - 30;
    int game_y = MENU_TITLE_X + 8;  // Position for first game
      // Draw "1: BREAKOUT" using custom letters
    draw_horizontal_text(game1_x, game_y, game1, MENU_TEXT_COLOR, 8);

    // Draw "2: PONG" using custom letters
    draw_horizontal_text(game2_x, game_y, game2, MENU_TEXT_COLOR, 8);
    
    // Draw "3: 1942" using custom letters  
    draw_horizontal_text(game3_x, game_y, game3, MENU_TEXT_COLOR, 8);
    
    // Draw "4: SNAKE" using custom letters
    draw_horizontal_text(game4_x, game_y, game4, MENU_TEXT_COLOR, 8);
    
    // Draw "5: TETRIS" using custom letters
    draw_horizontal_text(game5_x, game_y, game5, MENU_TEXT_COLOR, 8);
    
    // Draw "6: CAR RACE" using custom letters
    draw_horizontal_text(game6_x, game_y, game6, MENU_TEXT_COLOR, 8);
}

void main_menu_draw_sound_icon(bool selected, bool sound_on, uint16_t color)
{
    int x = MENU_SOUND_ICON_X;
    int y = MENU_SOUND_ICON_Y;
    int s = MENU_SOUND_ICON_SIZE;
    uint16_t border_color = selected ? MENU_SOUND_ICON_SELECTED_COLOR : MENU_SOUND_ICON_COLOR;
    // Draw border
	lcd_draw_rect(x, y, s, s, border_color);
    // Rotated icon (direction: right, through bottom)
    if (sound_on) {
        // Speaker with sound waves, rotated 90 deg right
    	lcd_draw_line(x + 8, y + 6, x + 16, y + 6, color); //
        lcd_draw_line(x + 8, y + 7, x + 4, y + 13, color); // Top
        lcd_draw_line(x + 16, y + 7, x + 20, y + 13, color); // Bottom
        lcd_draw_line(x + 4, y + 13, x + 20, y + 13, color); // Side
        // Sound waves (vertical)
    	lcd_draw_line(x + 10, y + 16, x + 14, y + 16, (color==MENU_BACKGROUND_COLOR) ? MENU_BACKGROUND_COLOR : GREEN);
    	lcd_draw_line(x + 8, y + 18, x + 16, y + 18, (color==MENU_BACKGROUND_COLOR) ? MENU_BACKGROUND_COLOR : GREEN);
    	lcd_draw_line(x + 6, y + 20, x + 18, y + 20, (color==MENU_BACKGROUND_COLOR) ? MENU_BACKGROUND_COLOR : GREEN);
    } else {
        // Speaker with X (muted), rotated 90 deg right
    	lcd_draw_line(x + 8, y + 6, x + 16, y + 6, color); //
        lcd_draw_line(x + 8, y + 7, x + 4, y + 13, color); // Top
        lcd_draw_line(x + 16, y + 7, x + 20, y + 13, color); // Bottom
        lcd_draw_line(x + 4, y + 13, x + 20, y + 13, color); // Side
        // X mark (vertical)
        lcd_draw_line(x + 6, y + 16, x + 18, y + 19, (color==MENU_BACKGROUND_COLOR) ? MENU_BACKGROUND_COLOR : RED);
        lcd_draw_line(x + 18, y + 16, x + 6, y + 19, (color==MENU_BACKGROUND_COLOR) ? MENU_BACKGROUND_COLOR : RED);
    }
}

void main_menu_draw_selection_box(void)
{
    // Clear previous selection areas first with thick RED rectangles
    for (int i = 0; i < MENU_TOTAL_SELECTIONS; i++) {
        int box_x, box_y;
        if (i == MENU_SOUND_SELECTION) {
            box_x = MENU_SOUND_ICON_X - 2;
            box_y = MENU_SOUND_ICON_Y - 2;
        } else {
            int game_idx = i - 1;
            box_x = MENU_TITLE_Y - (53 + (game_idx * 30) - 5); // Match game positions
            box_y = MENU_TITLE_X;
        }
        for (int thickness = 0; thickness < 3; thickness++) {
            lcd_draw_rect(box_x + thickness, box_y + thickness,
                          (i == MENU_SOUND_SELECTION ? MENU_SOUND_ICON_SIZE + 4 : 20) - (2 * thickness),
                          (i == MENU_SOUND_SELECTION ? MENU_SOUND_ICON_SIZE + 4 : 180) - (2 * thickness),
                          MENU_BACKGROUND_COLOR);
        }
    }
    // Draw selection box around current selection
    int box_x, box_y, box_w, box_h;
    if (menu_selection == MENU_SOUND_SELECTION) {
        box_x = MENU_SOUND_ICON_X - 2;
        box_y = MENU_SOUND_ICON_Y - 2;
        box_w = MENU_SOUND_ICON_SIZE + 4;
        box_h = MENU_SOUND_ICON_SIZE + 4;
    } else {
        int game_idx = menu_selection - 1;
        box_x = MENU_TITLE_Y - (53 + (game_idx * 30) - 5);
        box_y = MENU_TITLE_X;
        box_w = 20;
        box_h = 180;
    }
    for (int thickness = 0; thickness < 3; thickness++) {
        lcd_draw_rect(box_x + thickness, box_y + thickness,
                      box_w - (2 * thickness), box_h - (2 * thickness), MENU_SELECTION_COLOR);
    }
}


void main_menu_draw(void)
{
    lcd_clear_screen(MENU_BACKGROUND_COLOR);
    // Draw decorative border for horizontal display
    lcd_draw_rect(5, 5, LCD_WIDTH - 10, LCD_HEIGHT - 10, WHITE);
    lcd_draw_rect(6, 6, LCD_WIDTH - 12, LCD_HEIGHT - 12, WHITE);
    main_menu_draw_title();
    // Draw a line under the title for horizontal layout
    int line_x = MENU_TITLE_Y - 20;
    lcd_draw_line(line_x, 57, line_x, LCD_HEIGHT - 50, MENU_TITLE_COLOR); // Use LCD_HEIGHT as width
    main_menu_draw_games();
    main_menu_draw_selection_box();
    // Draw sound icon (selected if menu_selection == MENU_SOUND_SELECTION)
    main_menu_draw_sound_icon(menu_selection == MENU_SOUND_SELECTION, sound_on, BLUE);
    // Draw instruction at bottom for horizontal display
    const char* instruction = "Use joystick UP/DOWN to select, press button to play";
    int inst_length = 52;
    int char_width = 6;
    int inst_x = (LCD_HEIGHT - (inst_length * char_width)) / 2; // Use LCD_HEIGHT as width
    for (int i = 0; i < inst_length; i++) {
        char single_char[2] = {instruction[i], '\0'};
        lcd_display_string(inst_x + (i * char_width), LCD_WIDTH - 30, // Use LCD_WIDTH as height
                          (const uint8_t*)single_char, 10, CYAN);
    }
}

void main_menu_update_selection(int direction)
{
    if (direction == JOY_UP) {
        menu_selection = (menu_selection == 0) ? (MENU_TOTAL_SELECTIONS - 1) : (menu_selection - 1);
    } else if (direction == JOY_DOWN) {
        menu_selection = (menu_selection + 1) % MENU_TOTAL_SELECTIONS;
    }
    // Play selection sound only if both previous and new selection are games (not sound icon)
    if (is_sound_enabled()) {
        play_tone_conditional(NOTE_E7, 60); // Short beep
    }
    main_menu_draw_selection_box();
}

void main_menu_run(void)
{
    main_menu_init();

    while (menu_active) {
        int joystick1_direction = JOY_NONE;
        int joystick2_direction = JOY_NONE;
        Joystick_ReadBothDirections(&joystick1_direction, &joystick2_direction);
        if (joystick1_direction == JOY_UP || joystick2_direction == JOY_UP) {
            main_menu_update_selection(JOY_UP);
            HAL_Delay(200);
        } else if (joystick1_direction == JOY_DOWN || joystick2_direction == JOY_DOWN) {
            main_menu_update_selection(JOY_DOWN);
            HAL_Delay(200);
        }
        if (Joystick_ReadButton(JOYSTICK_1) || Joystick_ReadButton(JOYSTICK_2)) {
            if (menu_selection == MENU_SOUND_SELECTION) {
                sound_on = !sound_on;
                set_sound_enabled(sound_on);
                main_menu_draw_sound_icon(true, !sound_on, MENU_BACKGROUND_COLOR);
                main_menu_draw_sound_icon(true, sound_on, BLUE);
                HAL_Delay(300);
            } else {
                menu_active = false;
                switch (menu_selection - 1) {
                    case GAME_BREAKOUT:
                        breakout_play(10);
                        break;
                    case GAME_PONG:
                        pong_play();
                        break;
                    case GAME_1942:
                        game1942_play();
                        break;
                    case GAME_SNAKE:
                        snake_play();
                        break;
                    case GAME_TETRIS:
                        tetris_play();
                        break;
                    case 5:
                        carrace_play();
                        break;
                    default:
                        break;
                }
            }
        }
        HAL_Delay(50);
        if(!menu_active){
			menu_active = true;
			main_menu_init();
			uint32_t last_tick = HAL_GetTick();
			while (1) {
				uint32_t now = HAL_GetTick();
				if (now - last_tick >= 100) {
					break;
				}
			}
		}
    }
}





