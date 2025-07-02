#include "snakedemo.h"
#include "lcd_driver.h"
#include <stdlib.h>

// Grid ve hücre ayarları
#define CELL_SIZE 10
#define GRID_WIDTH (LCD_WIDTH / CELL_SIZE)
#define GRID_HEIGHT (LCD_HEIGHT / CELL_SIZE)
#define MAX_SNAKE_LENGTH 100

// Yılanın verileri
Point snake[MAX_SNAKE_LENGTH];
int snake_length = 5;
int dx = 1;
int dy = 0;

void draw_cell(int x, int y, uint16_t color) {
    lcd_fill_rect(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE, color);
}

void change_direction_randomly(void) {
    int dir = rand() % 2;
    switch (dir) {
        case 0: dx = 1; dy = 0; break;   // sağ
//        case 1: dx = -1; dy = 0; break;  // sol
        case 1: dx = 0; dy = 1; break;   // aşağı
//        case 3: dx = 0; dy = -1; break;  // yukarı
    }
}

void init_snake(void) {
    snake_length = 5;
    dx = 1;
    dy = 0;
    for (int i = 0; i < snake_length; i++) {
        snake[i].x = 5 - i;
        snake[i].y = 5;
        draw_cell(snake[i].x, snake[i].y, GREEN);
    }
}

void move_snake(void) {
    // Kuyruğu temizle
    draw_cell(snake[snake_length - 1].x, snake[snake_length - 1].y, BLACK);

    // Gövdeyi kaydır
    for (int i = snake_length - 1; i > 0; i--) {
        snake[i] = snake[i - 1];
    }

    // Yeni yön seç
    change_direction_randomly();

    // Yeni baş pozisyonunu hesapla
    snake[0].x += dx;
    snake[0].y += dy;

    // Ekran dışına çıkmayı önle (wrap-around)
    if (snake[0].x >= GRID_WIDTH) snake[0].x = 0;
    if (snake[0].x < 0) snake[0].x = GRID_WIDTH - 1;
    if (snake[0].y >= GRID_HEIGHT) snake[0].y = 0;
    if (snake[0].y < 0) snake[0].y = GRID_HEIGHT - 1;

    // Yeni başı çiz
    draw_cell(snake[0].x, snake[0].y, GREEN);

    // Rastgele büyüme
    if (rand() % 10 == 0 && snake_length < MAX_SNAKE_LENGTH) {
        snake[snake_length] = snake[snake_length - 1];  // uzat
        snake_length++;
    }
}
