#ifndef __PLAYMARIOSOUND_H
#define __PLAYMARIOSOUND_H

#include "main.h" // or replace with appropriate HAL include if outside CubeMX

// Note frequencies (in Hz)
#define NOTE_E6  1319
#define NOTE_G6  1568
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_D7  2349
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_G7  3136
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_E8  5274
#define NOTE_G8  6272
#define NOTE_A8  7040
#define REST     0

#ifdef __cplusplus
extern "C" {
#endif

// Call this in main to play the Mario melody
void play_mario(void);

// Plays a single tone
void play_tone(uint32_t freq, uint32_t duration_ms);

// Add prototypes for sound control
void set_sound_enabled(int enabled);
int is_sound_enabled(void);
void play_tone_conditional(uint32_t freq, uint32_t duration_ms);

#ifdef __cplusplus
}
#endif

#endif // __PLAYMARIOSOUND_H

// Only redefine play_tone in user/game files, not here!
#ifndef PLAYMARIOSOUND_C
#define play_tone(freq, dur) play_tone_conditional(freq, dur)
#endif
