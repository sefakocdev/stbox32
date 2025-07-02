#ifndef TETRIS_H
#define TETRIS_H

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "main.h"
#include "lcd_driver.h"
#include "joystick.h"
#include "playmariosound.h"

// Additional note definitions for Tetris sounds
#define NOTE_C3  131
#define NOTE_D3  147
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_G3  196
#define NOTE_A3  220
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_G5  784
#define NOTE_A5  880
#define NOTE_B5  988
#define NOTE_C6  1047

// Game field dimensions
#define TETRIS_FIELD_WIDTH 10
#define TETRIS_FIELD_HEIGHT 20
#define TETRIS_PREVIEW_HEIGHT 4  // Extra rows at top for piece spawning

// Display settings
#define TETRIS_BLOCK_SIZE 12
#define TETRIS_FIELD_X 20
#define TETRIS_FIELD_Y 20
#define TETRIS_NEXT_X 155
#define TETRIS_NEXT_Y 50

// Colors for different pieces
#define TETRIS_EMPTY_COLOR BLACK
#define TETRIS_BORDER_COLOR WHITE
#define TETRIS_I_COLOR CYAN
#define TETRIS_O_COLOR YELLOW
#define TETRIS_T_COLOR MAGENTA
#define TETRIS_S_COLOR GREEN
#define TETRIS_Z_COLOR RED
#define TETRIS_J_COLOR BLUE
#define TETRIS_L_COLOR ORANGE

// Piece types
typedef enum {
    TETRIS_PIECE_I = 0,
    TETRIS_PIECE_O = 1,
    TETRIS_PIECE_T = 2,
    TETRIS_PIECE_S = 3,
    TETRIS_PIECE_Z = 4,
    TETRIS_PIECE_J = 5,
    TETRIS_PIECE_L = 6,
    TETRIS_PIECE_COUNT = 7
} TetrisPieceType;

// Game states
typedef enum {
    TETRIS_STATE_PLAYING,
    TETRIS_STATE_LINE_CLEAR,
    TETRIS_STATE_GAME_OVER,
    TETRIS_STATE_PAUSED
} TetrisGameState;

// Tetris piece structure
typedef struct {
    int8_t blocks[4][2];  // 4 blocks, each with x,y coordinates
    uint16_t color;
    TetrisPieceType type;
} TetrisPiece;

// Current falling piece
typedef struct {
    TetrisPiece piece;
    int x, y;          // Position on field
    int rotation;      // Current rotation (0-3)
} TetrisFallingPiece;

// Game state
typedef struct {
    uint8_t field[TETRIS_FIELD_HEIGHT + TETRIS_PREVIEW_HEIGHT][TETRIS_FIELD_WIDTH];
    TetrisFallingPiece current_piece;
    TetrisPieceType next_piece_type;
    TetrisGameState state;    unsigned long score;
    unsigned long lines_cleared;
    uint8_t level;
    unsigned long drop_timer;
    unsigned long drop_interval;
    bool game_over;
    bool needs_redraw;
} TetrisGame;

// Function declarations
void tetris_init(void);
void tetris_play(void);
void tetris_reset_game(void);
void tetris_update(void);
void tetris_draw_border(void);
void tetris_draw_field(void);
void tetris_draw_piece(const TetrisFallingPiece* piece, uint16_t color);
void tetris_erase_piece(const TetrisFallingPiece* piece);
void tetris_draw_next_piece(void);
void tetris_draw_ui_labels(void);
void tetris_draw_ui_values(void);
void tetris_handle_input(void);

// Piece management
void tetris_spawn_piece(void);
bool tetris_can_move(const TetrisFallingPiece* piece, int dx, int dy, int new_rotation);
void tetris_move_piece(int dx, int dy);
void tetris_rotate_piece(void);
void tetris_lock_piece(void);
void tetris_clear_lines(void);
bool tetris_check_game_over(void);

// Utility functions
void tetris_get_piece_template(TetrisPieceType type, int rotation, TetrisPiece* piece);
void tetris_draw_block(int x, int y, uint16_t color);
uint16_t tetris_get_piece_color(TetrisPieceType type);

#endif // TETRIS_H
