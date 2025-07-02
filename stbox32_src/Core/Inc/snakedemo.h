#ifndef SNAKEDEMO_H
#define SNAKEDEMO_H

#include <stdint.h>

typedef struct {
    int x;
    int y;
} Point;

void init_snake(void);
void move_snake(void);
void change_direction_randomly(void);
void draw_cell(int x, int y, uint16_t color);

#endif
