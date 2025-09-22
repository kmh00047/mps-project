// I used a software based frequency calculation for the sound note.
// The reason is that the delay.h built in library requires a constant value
// of frequency at the compile time and the frequency can't be changed at runtime.
// But in our case, different events outputs different frequency.

#include <avr/io.h>
#include "sound.h"

// Simple software delay loop , approx 1 µs at 16MHz
static void delay_us_approx(uint16_t us)
{
	while (us--)
	{
		// 8 NOPs = 8 CPU cycles at 16 MHz = 0.5 us
		__asm__ __volatile__(
		"nop\n\t""nop\n\t""nop\n\t""nop\n\t"
		"nop\n\t""nop\n\t""nop\n\t""nop\n\t"
		);
	}
}

void sound_init(void)
{
	// Set PD3 as output
	DDRD |= (1 << PD3);
	// Initially turn it off
	PORTD &= ~(1 << PD3);
}

void play_tone(uint16_t frequency, uint16_t duration_ms)
{
	if (frequency == 0) // no sound
	{
		stop_tone();
		return;
	}

	uint32_t period_us = 1000000UL / frequency;
	uint32_t cycles = ((uint32_t)duration_ms * 1000UL) / period_us;

	for (uint32_t i = 0; i < cycles; i++)
	{
		PORTD |= (1 << PD3);  // PD3 HIGH
		delay_us_approx(period_us / 2);
		PORTD &= ~(1 << PD3); // PD3 LOW
		delay_us_approx(period_us / 2);
	}

	stop_tone();
}

void stop_tone(void)
{
	PORTD &= ~(1 << PD3); // Turn off buzzer
}