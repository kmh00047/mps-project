# MPS Project – AVR Mini Game

A fun little **catch-the-gift** game built for the **ATmega microcontroller** with an **SH1106 OLED display** and a **buzzer** for sound feedback.  
The player controls a platform to catch falling gifts, while score and lives are displayed on-screen. Includes **custom 3x6 bitmap fonts**, simple **sound effects**, and complete **game logic**.

---

## Features

- **Graphics**: Custom 3×6 pixel font rendering on SH1106 OLED (128×64).  
- **Sound**: Software-driven tone generation via buzzer (PD3).  
- **Gameplay**: 
  - Move platform left/right with buttons.  
  - Catch falling gifts to score points.  
  - Miss 3 gifts → game resets.  
- **HUD**: Displays live score and remaining lives.  
- **Reset**: Dedicated button to restart the game anytime.  
- **Hardware-friendly**: Includes TWI/I²C timeout handling to prevent lockups.

---

## Controls

| Button | Action             |
|--------|--------------------|
| Left   | Move platform left |
| Right  | Move platform right|
| Reset  | Restart the game   |

---

## Hardware Requirements

- **Microcontroller**: ATmega (tested at 16 MHz)  
- **Display**: 1.3" SH1106 OLED (I²C interface, address `0x78`)  
- **Sound**: Passive buzzer on **PD3**  
- **Buttons**: 3 push-buttons on **PC0, PC1, PC2**  
- **Power**: 5V supply (typical for ATmega dev boards)

**Pin connections:**

| Peripheral      | MCU Pin |
|-----------------|---------|
| OLED SDA        | SDA (PC4 on ATmega328P) |
| OLED SCL        | SCL (PC5 on ATmega328P) |
| Buzzer          | PD3 |
| Button Left     | PC0 |
| Button Right    | PC1 |
| Button Reset    | PC2 |

---

## Project Structure
```
mps-project/
├── font.h # 3x6 pixel font and text rendering functions
├── sound.h # Sound driver (declarations)
├── sound.c # Software-driven tone generation
├── main.c # Game logic, input handling, display control
```

---

## How It Works

1. **Display Buffering**:  
   A 1 KB buffer (`uint8_t buffer[1024]`) represents the OLED’s pixels.  
   Functions like `SH1106_drawPixel()` and `draw_text()` update the buffer, then `SH1106_display()` pushes it to the screen.

2. **Game Loop**:  
   - Polls buttons → moves platform.  
   - Spawns gifts randomly → moves them downward.  
   - Collision detection with platform updates score/lives.  
   - Renders HUD + objects every frame (20 FPS).  

3. **Sound**:  
   - Uses a software loop for precise frequency generation.  
   - Plays different tones for events:  
     - Gift caught = 1 kHz blip  
     - Life lost = 0.8 kHz longer tone  
     - Reset = 1.5 kHz startup tone  

---

## Building & Flashing

### Toolchain
- [AVR-GCC](https://www.nongnu.org/avr-libc/)
- [AVRDUDE](https://www.nongnu.org/avrdude/)

### Compile
```bash
avr-gcc -mmcu=atmega328p -DF_CPU=16000000UL -Os -o mps.elf main.c sound.c
avr-objcopy -O ihex -R .eeprom mps.elf mps.hex
```
Flash (example for Arduino Uno on /dev/ttyUSB0)
```bash
avrdude -c arduino -p atmega328p -P /dev/ttyUSB0 -b115200 -U flash:w:mps.hex
```
---

### Docs (Screenshots / Video / Documentation)

### License

This project is open-sourced under the [MIT License](./MIT LICENSE).
Feel free to use, modify, and learn from it.

### Inspiration

Made as part of a degree project to combine embedded systems, graphics programming, and game logic in a fun way.


---
