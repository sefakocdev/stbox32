#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    JOYSTICK_1,
    JOYSTICK_2
} JoystickId;

#define JOY_NONE      0
#define JOY_UP        1
#define JOY_DOWN      2
#define JOY_LEFT      3
#define JOY_RIGHT     4
#define JOY_UP_LEFT   5
#define JOY_UP_RIGHT  6
#define JOY_DOWN_LEFT 7
#define JOY_DOWN_RIGHT 8

// Function declarations
void Joystick_Init(void);
int Joystick_ReadDirection(JoystickId id);
bool Joystick_ReadButton(JoystickId id);

// New helper functions
void Joystick_StartConversion(void);
void Joystick_WaitAndStopConversion(void);

// Optimized functions for reading both joysticks at once
void Joystick_ReadBothDirections(int* joy1_direction, int* joy2_direction);
void Joystick_ReadAllValues(uint16_t* joy1_x, uint16_t* joy1_y, uint16_t* joy2_x, uint16_t* joy2_y);

#endif
