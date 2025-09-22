#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdlib.h>
#include "font.h"
#include "sound.h"

#define BTN_LEFT   0
#define BTN_RIGHT  1
#define BTN_RESET  2

#define PLATFORM_Y       7
#define PLATFORM_WIDTH   10
#define SCREEN_WIDTH     128
#define SCREEN_HEIGHT    64
#define HUD_HEIGHT       10
#define PLATFORM_SPEED   2

#define SH1106_ADDR      0x78

// ----Game Stats----
volatile uint16_t score = 0;
volatile uint8_t lives = 3;
uint8_t platform_x = 59;
uint8_t gift_x = 0;
uint8_t gift_y = HUD_HEIGHT + 1;
bool gift_active = false;

// --- I2C (TWI) with timeout ---
#define TWI_TIMEOUT 10000 // For hardware glitches

// Wait for TWINT flag with timeout, return false if timeout hit
static bool TWI_wait(void) {
	uint16_t timeout = TWI_TIMEOUT;
	while (!(TWCR & (1 << TWINT))) {
		if (--timeout == 0) {
			// Timeout: Reset TWI hardware to recover
			TWCR = 0;
			TWCR = (1 << TWEN);
			return false;
		}
	}
	return true;
}

void TWI_init(void) {
	TWSR = 0x00;
	TWBR = 0x20; // 100kHz
	TWCR = (1 << TWEN);
}

bool TWI_start(void) {
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	return TWI_wait();
}

bool TWI_stop(void) {
	TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
	uint16_t timeout = TWI_TIMEOUT;
	while (TWCR & (1 << TWSTO)) {
		if (--timeout == 0) {
			TWCR = 0;
			TWCR = (1 << TWEN);
			return false;
		}
	}
	return true;
}

bool TWI_write(uint8_t data) {
	TWDR = data;
	TWCR = (1 << TWINT) | (1 << TWEN);
	return TWI_wait();
}

//  OLED

bool SH1106_cmd(uint8_t cmd) {
	if (!TWI_start()) return false;
	if (!TWI_write(SH1106_ADDR)) { TWI_stop(); return false; }
	if (!TWI_write(0x00)) { TWI_stop(); return false; }
	if (!TWI_write(cmd)) { TWI_stop(); return false; }
	if (!TWI_stop()) return false;
	return true;
}

bool SH1106_data(uint8_t data) {
	if (!TWI_start()) return false;
	if (!TWI_write(SH1106_ADDR)) { TWI_stop(); return false; }
	if (!TWI_write(0x40)) { TWI_stop(); return false; }
	if (!TWI_write(data)) { TWI_stop(); return false; }
	if (!TWI_stop()) return false;
	return true;
}

void SH1106_init(void) {
	_delay_ms(100);
	SH1106_cmd(0xAE);
	SH1106_cmd(0xD5); SH1106_cmd(0x80);
	SH1106_cmd(0xA8); SH1106_cmd(0x3F);
	SH1106_cmd(0xD3); SH1106_cmd(0x00);
	SH1106_cmd(0x40);
	SH1106_cmd(0xAD); SH1106_cmd(0x8B);
	SH1106_cmd(0xA1);
	SH1106_cmd(0xC8);
	SH1106_cmd(0xDA); SH1106_cmd(0x12);
	SH1106_cmd(0x81); SH1106_cmd(0xCF);
	SH1106_cmd(0xD9); SH1106_cmd(0xF1);
	SH1106_cmd(0xDB); SH1106_cmd(0x40);
	SH1106_cmd(0xA4);
	SH1106_cmd(0xA6);
	SH1106_cmd(0xAF);
}

void SH1106_setCursor(uint8_t page, uint8_t col) {
	col += 2; // SH1106 display offset, (2 pixls shifted than 1306)
	SH1106_cmd(0xB0 + page);
	SH1106_cmd(0x00 + (col & 0x0F));
	SH1106_cmd(0x10 + ((col >> 4) & 0x0F));
}

// 1kb buffer for OLED pixels As 8 x 128 = 1024

uint8_t buffer[1024] = {0};

void SH1106_drawPixel(uint8_t x, uint8_t y, bool on) {
	if (x >= 128 || y >= 64) return;
	uint16_t index = x + (y / 8) * 128;
	if (on) buffer[index] |= (1 << (y % 8));
	else    buffer[index] &= ~(1 << (y % 8));
}

void SH1106_clearBuffer(void) {
	for (uint16_t i = 0; i < 1024; i++) buffer[i] = 0x00;
}

void SH1106_display(void) {
	// Display with error handling
	for (uint8_t page = 0; page < 8; page++) {
		SH1106_setCursor(page, 0);
		for (uint8_t col = 0; col < 128; col++) {
			// If data writing fails, break to avoid hanging
			if (!SH1106_data(buffer[col + (page * 128)])) break;
		}
	}
}

// ----Input----
bool is_button_pressed(uint8_t pin) {
	return !(PINC & (1 << pin));
}

// ----Draw Game Objects----
void draw_platform(uint8_t x) {
	for (uint8_t i = 0; i < PLATFORM_WIDTH; i++)
	SH1106_drawPixel(x + i, PLATFORM_Y * 8 + 7, true);
}

void draw_gift(void) {
	SH1106_drawPixel(gift_x, gift_y, true);
}

void draw_hud_separator(void) {
	for (uint8_t x = 0; x < SCREEN_WIDTH; x++) {
		SH1106_drawPixel(x, HUD_HEIGHT - 1, true);
		SH1106_drawPixel(x, HUD_HEIGHT, true);
	}
}

// --- Game Logic ---
void spawn_gift(void) {
	gift_x = rand() % (SCREEN_WIDTH - 1);
	gift_y = HUD_HEIGHT + 1;
	gift_active = true;
}

void reset_game(void) {
	SH1106_clearBuffer();
	SH1106_display();

	platform_x = 59;
	gift_active = false;
	score = 0;
	lives = 3;
	play_tone(1500, 1000); // Play 1.5 kHz note for 1 sec

	while (is_button_pressed(BTN_RESET));
	_delay_ms(100);
}

// --- Main ---
int main(void)
{
	// Inputs
	DDRC &= ~((1 << BTN_LEFT) | (1 << BTN_RIGHT) | (1 << BTN_RESET));
	PORTC |= (1 << BTN_LEFT) | (1 << BTN_RIGHT) | (1 << BTN_RESET);

	sound_init(); // Enable sounds
	TWI_init(); // Enable TWI modules
	SH1106_init(); // Enable Display
	SH1106_clearBuffer(); // Clear the display buffer to remove garbage noise and blank the display
	SH1106_display(); // Display Game play menu
	reset_game(); // Reset Everything

	while (1)
	{
		// Controls
		if (is_button_pressed(BTN_LEFT) && platform_x > 0)
		platform_x -= PLATFORM_SPEED;
		if (is_button_pressed(BTN_RIGHT) && platform_x < SCREEN_WIDTH - PLATFORM_WIDTH)
		platform_x += PLATFORM_SPEED;
		if (is_button_pressed(BTN_RESET))
		reset_game();

		// Gift logic
		if (!gift_active)
		spawn_gift();
		else
		{
			gift_y++;
			if (gift_y >= SCREEN_HEIGHT - 1)
			{
				if (gift_y >= PLATFORM_Y * 8)
				{
					if (gift_x >= platform_x && gift_x <= platform_x + PLATFORM_WIDTH)
					{
						score++;
						play_tone(1000, 100); // Play 1 kHz note for 100 ms
					}
					else
					{
						if (--lives == 0)
						reset_game();
						else
						play_tone(800, 300); // Play 0.8 kHz note for 300 ms
					}
				}
				gift_active = false;
			}
		}

		// Drawing
		SH1106_clearBuffer(); // Clear buffer for use
		draw_hud_separator(); // Draw the stats separation line
		draw_text(0, 0, "SCORE"); // Draw "Score" text
		draw_score(30, 0, score); // Draw Score
		draw_text(80, 0, "LIVES"); // Draw "Lives" text
		draw_char(110, 0, '0' + lives);  // Draw lives
		draw_platform(platform_x); // Draw Player Platform
		if (gift_active)
		draw_gift(); // Draw Gift
		SH1106_display(); // Display Gameplay

		_delay_ms(50); // 50 ms delay (20 FPS)
	}

	return 0;
}
