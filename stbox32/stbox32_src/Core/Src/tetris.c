#include "tetris.h"
#include <stdio.h>

// Global game state
static TetrisGame game;

// Tetris piece templates (4 rotations each)
// Each piece is defined as 4 blocks with relative coordinates
static const int8_t piece_templates[TETRIS_PIECE_COUNT][4][4][2] = {
    // I piece
    {
        {{-1,0}, {0,0}, {1,0}, {2,0}},   // Horizontal
        {{0,-1}, {0,0}, {0,1}, {0,2}},   // Vertical
        {{-1,0}, {0,0}, {1,0}, {2,0}},   // Horizontal
        {{0,-1}, {0,0}, {0,1}, {0,2}}    // Vertical
    },
    // O piece (square - no rotation needed)
    {
        {{0,0}, {1,0}, {0,1}, {1,1}},
        {{0,0}, {1,0}, {0,1}, {1,1}},
        {{0,0}, {1,0}, {0,1}, {1,1}},
        {{0,0}, {1,0}, {0,1}, {1,1}}
    },
    // T piece
    {
        {{-1,0}, {0,0}, {1,0}, {0,-1}},  // T pointing up
        {{0,-1}, {0,0}, {0,1}, {1,0}},   // T pointing right
        {{-1,0}, {0,0}, {1,0}, {0,1}},   // T pointing down
        {{0,-1}, {0,0}, {0,1}, {-1,0}}   // T pointing left
    },
    // S piece
    {
        {{-1,0}, {0,0}, {0,-1}, {1,-1}},
        {{0,-1}, {0,0}, {1,0}, {1,1}},
        {{-1,0}, {0,0}, {0,-1}, {1,-1}},
        {{0,-1}, {0,0}, {1,0}, {1,1}}
    },
    // Z piece
    {
        {{-1,-1}, {0,-1}, {0,0}, {1,0}},
        {{0,0}, {0,-1}, {1,-1}, {1,-2}},
        {{-1,-1}, {0,-1}, {0,0}, {1,0}},
        {{0,0}, {0,-1}, {1,-1}, {1,-2}}
    },
    // J piece
    {
        {{-1,-1}, {-1,0}, {0,0}, {1,0}},
        {{0,-1}, {0,0}, {0,1}, {1,-1}},
        {{-1,0}, {0,0}, {1,0}, {1,1}},
        {{-1,1}, {0,-1}, {0,0}, {0,1}}
    },
    // L piece
    {
        {{-1,0}, {0,0}, {1,0}, {1,-1}},
        {{0,-1}, {0,0}, {0,1}, {1,1}},
        {{-1,1}, {-1,0}, {0,0}, {1,0}},
        {{-1,-1}, {0,-1}, {0,0}, {0,1}}
    }
};

void tetris_init(void)
{
    srand(HAL_GetTick());
    tetris_reset_game();
}

void tetris_reset_game(void)
{
    // Clear field
    for (int y = 0; y < TETRIS_FIELD_HEIGHT + TETRIS_PREVIEW_HEIGHT; y++) {
        for (int x = 0; x < TETRIS_FIELD_WIDTH; x++) {
            game.field[y][x] = 0;
        }
    }
    
    game.score = 0;
    game.lines_cleared = 0;
    game.level = 1;
    game.drop_timer = 0;
    game.drop_interval = 800; // 800ms initial drop interval
    game.game_over = false;
    game.state = TETRIS_STATE_PLAYING;
    game.needs_redraw = true;
    
    game.next_piece_type = rand() % TETRIS_PIECE_COUNT;
    tetris_spawn_piece();
      // Draw static elements once
    tetris_draw_border();
    tetris_draw_ui_labels();
    tetris_draw_next_piece();
    tetris_draw_ui_values();
    tetris_draw_field();
    
    // Draw the first piece
    tetris_draw_piece(&game.current_piece, game.current_piece.piece.color);
}

void tetris_play(void)
{
    lcd_clear_screen(BLACK);
    tetris_init();
    
    unsigned long last_time = HAL_GetTick();
    unsigned long input_delay = 0;
    
    while (!game.game_over) {
        unsigned long current_time = HAL_GetTick();
        unsigned long delta_time = current_time - last_time;
        last_time = current_time;
        
        // Handle input with delay
        if (input_delay == 0) {
            tetris_handle_input();
            input_delay = 0; // 150ms input delay
        } else if (input_delay > delta_time) {
            input_delay -= delta_time;
        } else {
            input_delay = 0;
        }
          // Update game logic
        game.drop_timer += delta_time;
        if (game.drop_timer >= game.drop_interval) {
            game.drop_timer = 0;
            if (!tetris_can_move(&game.current_piece, 0, 1, game.current_piece.rotation)) {
                tetris_lock_piece();
                
                // Check for game over immediately after locking piece
                if (tetris_check_game_over()) {
                    game.game_over = true;
                    game.state = TETRIS_STATE_GAME_OVER;
                } else {
                    tetris_clear_lines();
                    tetris_spawn_piece();
                    
                    // Redraw next piece only when new piece spawns
                    tetris_draw_next_piece();
                    
                    // Final check - if new piece can't be placed
                    if (!tetris_can_move(&game.current_piece, 0, 0, game.current_piece.rotation)) {
                        game.game_over = true;
                        game.state = TETRIS_STATE_GAME_OVER;
                    }
                }
            } else {
                // Erase piece at old position
                tetris_erase_piece(&game.current_piece);
                game.current_piece.y++;
                // Draw piece at new position
                tetris_draw_piece(&game.current_piece, game.current_piece.piece.color);
            }
        }
        
        HAL_Delay(10);
        
        // Check for exit
        if (Joystick_ReadButton(JOYSTICK_1) && Joystick_ReadButton(JOYSTICK_2)) {
            HAL_Delay(500);
            if (Joystick_ReadButton(JOYSTICK_1) && Joystick_ReadButton(JOYSTICK_2)) {
                break; // Exit game
            }
        }
    }

    // Game over screen
    if (game.game_over) {
    	lcd_clear_screen(BLACK);
        char score_str[32];
        sprintf(score_str, "GAME OVER");
        lcd_display_string(90, 150, (const uint8_t*)score_str, 16, RED);
        sprintf(score_str, "Score: %lu", game.score);
        lcd_display_string(80, 180, (const uint8_t*)score_str, 16, BLUE);
        sprintf(score_str, "Lines: %lu", game.lines_cleared);
        lcd_display_string(80, 200, (const uint8_t*)score_str, 16, GREEN);
        
        if (is_sound_enabled()) {
            play_tone_conditional(NOTE_C4, 500);
            HAL_Delay(100);
            play_tone_conditional(NOTE_G3, 500);
            HAL_Delay(100);
            play_tone_conditional(NOTE_E3, 1000);
        }
        
        HAL_Delay(3000);
    }
}

void tetris_spawn_piece(void)
{
    game.current_piece.piece.type = game.next_piece_type;
    game.current_piece.x = TETRIS_FIELD_WIDTH / 2;
    game.current_piece.y = TETRIS_PREVIEW_HEIGHT - 2;
    game.current_piece.rotation = 0;
    
    tetris_get_piece_template(game.current_piece.piece.type, 0, &game.current_piece.piece);
    
    game.next_piece_type = rand() % TETRIS_PIECE_COUNT;
}

bool tetris_can_move(const TetrisFallingPiece* piece, int dx, int dy, int new_rotation)
{
    TetrisPiece temp_piece;
    tetris_get_piece_template(piece->piece.type, new_rotation, &temp_piece);
    
    int new_x = piece->x + dx;
    int new_y = piece->y + dy;
    
    for (int i = 0; i < 4; i++) {
        int block_x = new_x + temp_piece.blocks[i][0];
        int block_y = new_y + temp_piece.blocks[i][1];
        
        // Check bounds
        if (block_x < 0 || block_x >= TETRIS_FIELD_WIDTH || 
            block_y >= TETRIS_FIELD_HEIGHT + TETRIS_PREVIEW_HEIGHT) {
            return false;
        }
        
        // Check collision with existing blocks (only below preview area)
        if (block_y >= TETRIS_PREVIEW_HEIGHT && game.field[block_y][block_x] != 0) {
            return false;
        }
    }
    
    return true;
}

void tetris_move_piece(int dx, int dy)
{
    if (tetris_can_move(&game.current_piece, dx, dy, game.current_piece.rotation)) {
        // Erase piece at old position
        tetris_erase_piece(&game.current_piece);
        
        game.current_piece.x += dx;
        game.current_piece.y += dy;
        
        // Draw piece at new position
        tetris_draw_piece(&game.current_piece, game.current_piece.piece.color);
        
        if (is_sound_enabled()) {
            play_tone_conditional(NOTE_C5, 50);
        }
    }
}

void tetris_rotate_piece(void)
{
    int new_rotation = (game.current_piece.rotation + 1) % 4;
    if (tetris_can_move(&game.current_piece, 0, 0, new_rotation)) {
        // Erase piece at old position
        tetris_erase_piece(&game.current_piece);
        
        game.current_piece.rotation = new_rotation;
        tetris_get_piece_template(game.current_piece.piece.type, new_rotation, &game.current_piece.piece);
        
        // Draw piece with new rotation
        tetris_draw_piece(&game.current_piece, game.current_piece.piece.color);
        
        if (is_sound_enabled()) {
            play_tone_conditional(NOTE_E5, 80);
        }
    }
}

void tetris_lock_piece(void)
{
    for (int i = 0; i < 4; i++) {
        int block_x = game.current_piece.x + game.current_piece.piece.blocks[i][0];
        int block_y = game.current_piece.y + game.current_piece.piece.blocks[i][1];
        
        if (block_x >= 0 && block_x < TETRIS_FIELD_WIDTH && 
            block_y >= 0 && block_y < TETRIS_FIELD_HEIGHT + TETRIS_PREVIEW_HEIGHT) {
            game.field[block_y][block_x] = game.current_piece.piece.type + 1;
        }
    }
    
    if (is_sound_enabled()) {
        play_tone_conditional(NOTE_G4, 150);
    }
}

void tetris_clear_lines(void)
{
    int lines_to_clear[4];
    int clear_count = 0;
    
    // Find complete lines
    for (int y = TETRIS_PREVIEW_HEIGHT; y < TETRIS_FIELD_HEIGHT + TETRIS_PREVIEW_HEIGHT; y++) {
        bool line_full = true;
        for (int x = 0; x < TETRIS_FIELD_WIDTH; x++) {
            if (game.field[y][x] == 0) {
                line_full = false;
                break;
            }
        }
        if (line_full) {
            lines_to_clear[clear_count++] = y;
        }
    }
    
    if (clear_count > 0) {
        // Clear the lines
        for (int i = 0; i < clear_count; i++) {
            int clear_y = lines_to_clear[i];
            // Move all lines above down
            for (int y = clear_y; y > TETRIS_PREVIEW_HEIGHT; y--) {
                for (int x = 0; x < TETRIS_FIELD_WIDTH; x++) {
                    game.field[y][x] = game.field[y-1][x];
                }
            }
            // Clear top line
            for (int x = 0; x < TETRIS_FIELD_WIDTH; x++) {
                game.field[TETRIS_PREVIEW_HEIGHT][x] = 0;
            }
            
            // Adjust remaining lines to clear
            for (int j = i + 1; j < clear_count; j++) {
                if (lines_to_clear[j] < clear_y) {
                    lines_to_clear[j]++;
                }
            }
        }
          // Update score and level
        unsigned long old_score = game.score;
        unsigned long old_lines = game.lines_cleared;
        uint8_t old_level = game.level;
        
        game.lines_cleared += clear_count;
        switch (clear_count) {
            case 1: game.score += 40 * game.level; break;
            case 2: game.score += 100 * game.level; break;
            case 3: game.score += 300 * game.level; break;
            case 4: game.score += 1200 * game.level; break;
        }
        
        // Level progression
        if (game.lines_cleared >= game.level * 10) {
            game.level++;
            if (game.drop_interval > 100) {
                game.drop_interval -= 50;
            }
        }
        
        // Redraw UI values only if they changed
        if (old_score != game.score || old_lines != game.lines_cleared || old_level != game.level) {
            tetris_draw_ui_values();
        }
          // Redraw the field to show cleared lines
        tetris_draw_field();
        
        // Check if clearing lines caused game over (pieces reached top)
        if (tetris_check_game_over()) {
            game.game_over = true;
            game.state = TETRIS_STATE_GAME_OVER;
            return; // Exit early to prevent sound effects
        }
        
        if (is_sound_enabled()) {
            switch (clear_count) {
                case 1:
                    play_tone_conditional(NOTE_C6, 200);
                    break;
                case 2:
                    play_tone_conditional(NOTE_E6, 200);
                    play_tone_conditional(NOTE_G6, 200);
                    break;
                case 3:
                    play_tone_conditional(NOTE_C6, 150);
                    play_tone_conditional(NOTE_E6, 150);
                    play_tone_conditional(NOTE_G6, 200);
                    break;
                case 4: // Tetris!
                    for (int i = 0; i < 8; i++) {
                        play_tone_conditional(NOTE_C7, 100);
                    }
                    break;
            }
        }
    }
}

void tetris_handle_input(void)
{
    int joy1_dir = JOY_NONE;
    int joy2_dir = JOY_NONE;
    
    Joystick_ReadBothDirections(&joy1_dir, &joy2_dir);
    
    // Use either joystick
    int direction = (joy1_dir != JOY_NONE) ? joy1_dir : joy2_dir;
    
    switch (direction) {
        case JOY_DOWN:
            tetris_move_piece(-1, 0);
            break;
        case JOY_UP:
            tetris_move_piece(1, 0);
            break;
        case JOY_RIGHT:
            tetris_move_piece(0, 1);
            char text[32];
            // Score
            sprintf(text, "Score: %lu", game.score);
            lcd_display_string(TETRIS_NEXT_X, TETRIS_NEXT_Y + 80, (const uint8_t*)text, 12, BLACK);
            game.score++; // Bonus for soft drop
            sprintf(text, "Score: %lu", game.score);
            lcd_display_string(TETRIS_NEXT_X, TETRIS_NEXT_Y + 80, (const uint8_t*)text, 12, WHITE);
            break;
        case JOY_LEFT:
            tetris_rotate_piece();
            break;
    }
}

void tetris_get_piece_template(TetrisPieceType type, int rotation, TetrisPiece* piece)
{
    piece->type = type;
    piece->color = tetris_get_piece_color(type);
    
    for (int i = 0; i < 4; i++) {
        piece->blocks[i][0] = piece_templates[type][rotation][i][0];
        piece->blocks[i][1] = piece_templates[type][rotation][i][1];
    }
}

uint16_t tetris_get_piece_color(TetrisPieceType type)
{
    switch (type) {
        case TETRIS_PIECE_I: return TETRIS_I_COLOR;
        case TETRIS_PIECE_O: return TETRIS_O_COLOR;
        case TETRIS_PIECE_T: return TETRIS_T_COLOR;
        case TETRIS_PIECE_S: return TETRIS_S_COLOR;
        case TETRIS_PIECE_Z: return TETRIS_Z_COLOR;
        case TETRIS_PIECE_J: return TETRIS_J_COLOR;
        case TETRIS_PIECE_L: return TETRIS_L_COLOR;
        default: return WHITE;
    }
}

void tetris_draw_border(void)
{
    // Draw border once - never changes
    lcd_draw_rect(TETRIS_FIELD_X - 2, TETRIS_FIELD_Y - 2, 
                  TETRIS_FIELD_WIDTH * TETRIS_BLOCK_SIZE + 4, 
                  TETRIS_FIELD_HEIGHT * TETRIS_BLOCK_SIZE + 4, 
                  TETRIS_BORDER_COLOR);
                  
    // Draw next piece border
    lcd_draw_rect(TETRIS_NEXT_X - 2, TETRIS_NEXT_Y - 2, 
                  4 * TETRIS_BLOCK_SIZE + 4, 4 * TETRIS_BLOCK_SIZE + 4, 
                  TETRIS_BORDER_COLOR);
}

void tetris_draw_ui_labels(void)
{
    // Draw static labels that never change
    lcd_display_string(TETRIS_NEXT_X, TETRIS_NEXT_Y - 20, (const uint8_t*)"NEXT", 16, WHITE);
    lcd_display_string(10, LCD_WIDTH - 40, (const uint8_t*)"UP:Rotate", 10, CYAN);
    lcd_display_string(10, LCD_WIDTH - 25, (const uint8_t*)"Both buttons: Exit", 10, CYAN);
}

void tetris_draw_ui_values(void)
{
    char text[32];
    
    // Clear old values by drawing black rectangles
    lcd_fill_rect(TETRIS_NEXT_X, TETRIS_NEXT_Y + 80, 83, 12, BLACK);
    lcd_fill_rect(TETRIS_NEXT_X, TETRIS_NEXT_Y + 100, 83, 12, BLACK);
    lcd_fill_rect(TETRIS_NEXT_X, TETRIS_NEXT_Y + 120, 83, 12, BLACK);
    
    // Score
    sprintf(text, "Score: %lu", game.score);
    lcd_display_string(TETRIS_NEXT_X, TETRIS_NEXT_Y + 80, (const uint8_t*)text, 12, WHITE);
    
    // Lines
    sprintf(text, "Lines: %lu", game.lines_cleared);
    lcd_display_string(TETRIS_NEXT_X, TETRIS_NEXT_Y + 100, (const uint8_t*)text, 12, WHITE);
    
    // Level
    sprintf(text, "Level: %d", game.level);
    lcd_display_string(TETRIS_NEXT_X, TETRIS_NEXT_Y + 120, (const uint8_t*)text, 12, WHITE);
}

void tetris_erase_piece(const TetrisFallingPiece* piece)
{
    for (int i = 0; i < 4; i++) {
        int block_x = piece->x + piece->piece.blocks[i][0];
        int block_y = piece->y + piece->piece.blocks[i][1];
        
        // Only erase blocks that are in the visible playfield
        if (block_x >= 0 && block_x < TETRIS_FIELD_WIDTH && 
            block_y >= TETRIS_PREVIEW_HEIGHT && 
            block_y < TETRIS_FIELD_HEIGHT + TETRIS_PREVIEW_HEIGHT) {
            
            int screen_y = block_y - TETRIS_PREVIEW_HEIGHT;
            
            // Check if there's a locked piece at this position
            uint16_t background_color = TETRIS_EMPTY_COLOR;
            if (game.field[block_y][block_x] != 0) {
                background_color = tetris_get_piece_color(game.field[block_y][block_x] - 1);
            }
            
            tetris_draw_block(TETRIS_FIELD_X + block_x * TETRIS_BLOCK_SIZE,
                             TETRIS_FIELD_Y + screen_y * TETRIS_BLOCK_SIZE,
                             background_color);
        }
    }
}

void tetris_draw_field(void)
{
    // Only redraw the playfield blocks (no border)
    for (int y = 0; y < TETRIS_FIELD_HEIGHT; y++) {
        for (int x = 0; x < TETRIS_FIELD_WIDTH; x++) {
            uint16_t color = TETRIS_EMPTY_COLOR;
            int field_y = y + TETRIS_PREVIEW_HEIGHT;
            
            if (game.field[field_y][x] != 0) {
                color = tetris_get_piece_color(game.field[field_y][x] - 1);
            }
            
            tetris_draw_block(TETRIS_FIELD_X + x * TETRIS_BLOCK_SIZE, 
                             TETRIS_FIELD_Y + y * TETRIS_BLOCK_SIZE, 
                             color);
        }
    }
}

void tetris_draw_piece(const TetrisFallingPiece* piece, uint16_t color)
{
    for (int i = 0; i < 4; i++) {
        int block_x = piece->x + piece->piece.blocks[i][0];
        int block_y = piece->y + piece->piece.blocks[i][1];
        
        // Only draw blocks that are in the visible playfield
        if (block_x >= 0 && block_x < TETRIS_FIELD_WIDTH && 
            block_y >= TETRIS_PREVIEW_HEIGHT && 
            block_y < TETRIS_FIELD_HEIGHT + TETRIS_PREVIEW_HEIGHT) {
            
            int screen_y = block_y - TETRIS_PREVIEW_HEIGHT;
            tetris_draw_block(TETRIS_FIELD_X + block_x * TETRIS_BLOCK_SIZE,
                             TETRIS_FIELD_Y + screen_y * TETRIS_BLOCK_SIZE,
                             color);
        }
    }
}

void tetris_draw_next_piece(void)
{
    // Clear next piece area
    lcd_fill_rect(TETRIS_NEXT_X, TETRIS_NEXT_Y, 
                  4 * TETRIS_BLOCK_SIZE, 4 * TETRIS_BLOCK_SIZE, BLACK);
    
    // Draw next piece
    TetrisPiece next_piece;
    tetris_get_piece_template(game.next_piece_type, 0, &next_piece);
    
    for (int i = 0; i < 4; i++) {
        int block_x = next_piece.blocks[i][0] + 2; // Center in preview area
        int block_y = next_piece.blocks[i][1] + 2;
        
        if (block_x >= 0 && block_x < 4 && block_y >= 0 && block_y < 4) {
            tetris_draw_block(TETRIS_NEXT_X -2 + block_x * TETRIS_BLOCK_SIZE,
                             TETRIS_NEXT_Y + block_y * TETRIS_BLOCK_SIZE,
                             next_piece.color);
        }
    }
}

void tetris_draw_block(int x, int y, uint16_t color)
{
    if (color == TETRIS_EMPTY_COLOR) {
    	lcd_fill_rect(x, y, TETRIS_BLOCK_SIZE - 1, TETRIS_BLOCK_SIZE - 1, BLACK);
    } else {
    	lcd_fill_rect(x, y, TETRIS_BLOCK_SIZE - 1, TETRIS_BLOCK_SIZE - 1, color);
    }
}

bool tetris_check_game_over(void)
{
    // Check if any locked pieces have reached the visible game area (top row of playfield)
    for (int x = 0; x < TETRIS_FIELD_WIDTH; x++) {
        if (game.field[TETRIS_PREVIEW_HEIGHT][x] != 0) {
            return true; // Game over - pieces reached the top visible line
        }
    }
    
    // Also check the row just above the visible area
    for (int x = 0; x < TETRIS_FIELD_WIDTH; x++) {
        if (game.field[TETRIS_PREVIEW_HEIGHT - 1][x] != 0) {
            return true; // Game over - pieces in preview area
        }
    }
    
    return false;
}
