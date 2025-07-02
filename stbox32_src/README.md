# STBOX32 - STM32 Mini Game Console

A feature-rich, portable mini game console built on the STM32F411RE microcontroller platform. This project demonstrates embedded systems design, real-time programming, and interactive gaming on a custom hardware platform.

## 🎮 Project Overview

STBOX32 is a compact gaming device that combines modern embedded systems technology with classic gaming experiences. The console features a color LCD display, dual analog joysticks, sound effects, and a custom 3D-printed enclosure, offering an engaging platform for both educational and recreational purposes.

### 🎬 Demo Video
[![STBOX32 Demo](https://img.youtube.com/vi/dF-4JCDcyiY/0.jpg)](https://youtu.be/dF-4JCDcyiY)

## ✨ Features

- **Six Built-in Games:**
  - 🧱 **Breakout** - Classic brick-breaking game
  - 🏓 **Pong** - Two-player table tennis game
  - ✈️ **1942** - Top-down shooter game
  - 🐍 **Snake** - Classic snake game with growing mechanics
  - 🧩 **Tetris** - Block-stacking puzzle game
  - 🏎️ **Car Race** - Two-player racing game

- **Hardware Features:**
  - 2.8" Color TFT LCD Display (240x320, ST7789V controller)
  - Dual analog joysticks with integrated push buttons
  - Buzzer for sound effects and music
  - Reset button for system restart
  - Battery-powered operation with on/off switch
  - Custom 3D-printed enclosure

## 🔧 Hardware Architecture

### Core Components
- **Microcontroller:** STM32F411RET6 (ARM Cortex-M4, 100MHz)
- **Display:** 2.8" TFT LCD (240x320 pixels) with ST7789V controller
- **Input:** Two analog joysticks (X/Y axes + push buttons)
- **Audio:** Piezo buzzer for sound effects
- **Power:** Battery pack with power switch
- **Enclosure:** Custom 3D-printed case

### Communication Interfaces
- **SPI:** LCD display communication
- **ADC:** Joystick analog input reading
- **GPIO:** Button inputs and control signals
- **PWM:** Buzzer control for sound generation

## 🏗️ Software Architecture

### Core System
- **Real-time Control:** HAL-based peripheral management
- **Graphics Engine:** Custom SPI-optimized drawing routines
- **Input System:** Analog joystick processing with direction mapping
- **Sound System:** PWM-based tone generation
- **Menu System:** Interactive game selection interface

### Performance Optimizations
Due to SPI interface limitations and lack of internal framebuffer:
- **Selective Redrawing:** Only modified screen regions are updated
- **Minimized Fill Operations:** Large filled areas are avoided for better performance
- **Efficient Drawing Routines:** Custom functions minimize SPI transactions
- **Real-time Optimization:** Graphics rendering optimized for smooth gameplay

### Code Structure
```
Core/
├── Src/
│   ├── main.c              # System initialization and main loop
│   ├── main_menu.c         # Menu system and game selection
│   ├── joystick.c          # Analog input processing
│   ├── breakout.c          # Breakout game implementation
│   ├── pong.c              # Pong game implementation
│   ├── game1942.c          # 1942 shooter game
│   ├── snake.c             # Snake game implementation
│   ├── tetris.c            # Tetris game implementation
│   ├── carrace.c           # Car racing game
│   └── playmariosound.c    # Sound effects and music
├── Inc/                    # Header files
└── Startup/                # STM32 startup code

Drivers/
├── LCD/
│   ├── lcd_driver.c        # ST7789V display driver
│   └── lcd_driver.h        # Display function declarations
├── Fonts/                  # Font rendering support
└── STM32F4xx_HAL_Driver/   # STM32 HAL libraries
```

### Prerequisites
- **STM32CubeIDE** or compatible ARM GCC toolchain
- **STM32F411RE** development board (NUCLEO-F411RE recommended)
- **Hardware components** as listed in the hardware section

### Hardware Setup
1. Connect the 2.8" TFT LCD to SPI1 (pins specified in `main.h`)
2. Wire analog joysticks to ADC channels (PA0, PA1, PA4, PB0)
3. Connect buzzer to PWM-capable pin (TIM3_CH2)
4. Add push buttons and power switch as specified
5. Assemble in 3D-printed enclosure

## 🎯 Usage

1. **Power On:** Use the power switch to turn on the device
2. **Main Menu:** Navigate through games using the joystick
3. **Game Selection:** Press joystick button to select a game
4. **Gameplay:** Use dual joysticks for game control
5. **Sound Toggle:** Option available in menu to enable/disable sound
6. **Reset:** Use hardware reset button to restart system and returns to main menu

## 🎮 Games Description

### Breakout
Classic brick-breaking game where players control a paddle to bounce a ball and destroy bricks. Features multiple levels with increasing difficulty.

### Pong
Two-player table tennis simulation. Each player controls a paddle to hit the ball back and forth. First to reach the score limit wins.

### 1942
Top-down scrolling shooter where players control an aircraft, avoiding enemies and obstacles while shooting targets.

### Snake
The classic snake game where players guide a growing snake to eat food while avoiding walls and the snake's own body.

### Tetris
Block-stacking puzzle game where players arrange falling tetromino pieces to complete horizontal lines.

### Car Race
Racing game where players navigate a car through obstacles and other vehicles on a scrolling track.

## 🔧 Development Notes

### Graphics Limitations
- **SPI Speed Constraint:** Display updates are limited by SPI interface speed
- **No Framebuffer:** Each pixel must be sent directly to display
- **DMA Challenges:** ST7789V command structure makes DMA implementation complex

### Optimization Strategies
- **Selective Updates:** Only changed screen regions are redrawn
- **Efficient Primitives:** Custom drawing functions minimize SPI overhead
- **Smart Rendering:** Avoid large filled areas, prefer smaller objects

### Future Improvements
- DMA implementation for faster graphics
- Additional games and features
- Enhanced sound system with multiple channels
- Wireless connectivity options
- Save game functionality

*This project demonstrates the power of modern embedded systems in creating engaging, interactive devices that combine hardware design, software engineering, and user experience design.*
