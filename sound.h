#ifndef SOUND_H
#define SOUND_H

#include <stdint.h>

void sound_init(void);
void play_tone(uint16_t frequency, uint16_t duration_ms);
void stop_tone(void);

#endif
