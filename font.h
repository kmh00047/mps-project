// This segment of Code for defination of Characters is written by Chatgpt

#ifndef FONT_H
#define FONT_H

#include <stdint.h>
#include <stdbool.h>

// 3x6 font: Each char is 3 bytes (3 columns of 6 bits)
const uint8_t font3x6[][3] = {
    // 0-9
    {0x3E, 0x51, 0x3E}, // 0
    {0x22, 0x7F, 0x20}, // 1
    {0x72, 0x49, 0x46}, // 2
    {0x41, 0x49, 0x36}, // 3
    {0x0F, 0x08, 0x7F}, // 4
    {0x4F, 0x49, 0x31}, // 5
    {0x3E, 0x49, 0x32}, // 6
    {0x03, 0x01, 0x7F}, // 7
    {0x36, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x3E}, // 9

    // A-Z
    {0x7E, 0x09, 0x7E}, // A
    {0x7F, 0x49, 0x36}, // B
    {0x3E, 0x41, 0x22}, // C
    {0x7F, 0x41, 0x3E}, // D
    {0x7F, 0x49, 0x41}, // E
    {0x7F, 0x09, 0x01}, // F
    {0x3E, 0x41, 0x79}, // G
    {0x7F, 0x08, 0x7F}, // H
    {0x41, 0x7F, 0x41}, // I
    {0x20, 0x40, 0x3F}, // J
    {0x7F, 0x08, 0x77}, // K
    {0x7F, 0x40, 0x40}, // L
    {0x7F, 0x02, 0x7F}, // M
    {0x7F, 0x04, 0x7F}, // N
    {0x3E, 0x41, 0x3E}, // O
    {0x7F, 0x09, 0x06}, // P
    {0x3E, 0x41, 0xBE}, // Q
    {0x7F, 0x09, 0x76}, // R
    {0x26, 0x49, 0x32}, // S
    {0x01, 0x7F, 0x01}, // T
    {0x3F, 0x40, 0x3F}, // U
    {0x1F, 0x60, 0x1F}, // V
    {0x7F, 0x10, 0x7F}, // W
    {0x77, 0x08, 0x77}, // X
    {0x07, 0x78, 0x07}, // Y
    {0x71, 0x49, 0x47}  // Z
};

// Draw a single 3x6 character
void draw_char(uint8_t x, uint8_t y, char c)
{
    uint8_t index;
    
    if (c >= '0' && c <= '9')
        index = c - '0';
    else if (c >= 'A' && c <= 'Z')
        index = c - 'A' + 10;
    else
        return; // Unsupported char, so return

    for (uint8_t col = 0; col < 3; col++)
    {
        uint8_t column = font3x6[index][col];
        for (uint8_t row = 0; row <= 6; row++)
        {
            bool pixel_on = column & (1 << row);
            SH1106_drawPixel(x + col, y + row, pixel_on);
        }
    }
}


// Draw a string using draw_char, each letter spaced 1 pixal apart
void draw_text(uint8_t x, uint8_t y, const char* str)
{
    while (*str)
    {
        draw_char(x, y, *str);
        x += 4; // 3px width + 1px spacing
        str++;
    }
}

#endif

