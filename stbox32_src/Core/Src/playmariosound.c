#define PLAYMARIOSOUND_C
#include "playmariosound.h"

extern TIM_HandleTypeDef htim3; // Ensure htim3 is configured for PWM elsewhere

// Complete Mario melody
int melody[] = {
    NOTE_E7, NOTE_E7, REST, NOTE_E7,
    REST, NOTE_C7, NOTE_E7, REST,
    NOTE_G7, REST, REST, REST,
    NOTE_G6, REST, REST, REST,

    NOTE_C7, REST, REST, NOTE_G6,
    REST, REST, NOTE_E6, REST,
    REST, NOTE_A6, REST, NOTE_B6,
    REST, NOTE_AS6, NOTE_A6,

    NOTE_G6, NOTE_E7, NOTE_G7,
    NOTE_A7, REST, NOTE_F7, NOTE_G7,
    REST, NOTE_E7, REST, NOTE_C7,
    NOTE_D7, NOTE_B6,

    REST, REST,
    NOTE_C7, REST, REST, NOTE_G6,
    REST, REST, NOTE_E6, REST,
    REST, NOTE_A6, REST, NOTE_B6,
    REST, NOTE_AS6, NOTE_A6
};

int noteDurations[] = {
    8, 8, 8, 8,
    8, 8, 4, 8,
    4, 8, 8, 8,
    4, 8, 8, 8,

    4, 8, 8, 4,
    8, 8, 4, 8,
    8, 4, 8, 4,
    8, 8, 4,

    4, 8, 4,
    4, 8, 4, 4,
    8, 4, 8, 4,
    4, 4,

    8, 8,
    4, 8, 8, 4,
    8, 8, 4, 8,
    8, 4, 8, 4,
    8, 8, 4
};

// --- Sound control global ---
volatile int sound_enabled = 1;
void set_sound_enabled(int enabled) { sound_enabled = enabled; }
int is_sound_enabled(void) { return sound_enabled; }
// --- Wrapper for play_tone ---
void play_tone_conditional(uint32_t freq, uint32_t duration_ms) {
    if (sound_enabled) play_tone(freq, duration_ms);
    else HAL_Delay(duration_ms + 20); // Simulate delay if muted
}

void play_tone(uint32_t freq, uint32_t duration_ms)
{
    if (freq == REST) {
        HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_2);
        HAL_Delay(duration_ms);
        return;
    }

    uint32_t arr = 1000000 / freq - 1; // 1 MHz timer base frequency
    __HAL_TIM_SET_AUTORELOAD(&htim3, arr);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, arr / 2); // 50% duty

    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    HAL_Delay(duration_ms);
    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_2);
    HAL_Delay(20); // short delay between notes
}

void play_mario(void)
{
    int length = sizeof(melody) / sizeof(melody[0]);
    for (int i = 0; i < length; i++) {
        int noteDuration = 1000 / noteDurations[i]; // e.g., 4 = quarter note
        play_tone(melody[i], noteDuration);
    }
}
