# 🎮 ESP32 ILI9488 Game Engine & UI Framework

A high-performance, hardware-accelerated C++20 game engine and touch UI framework for **ESP32** microcontrollers paired with an **ILI9488 480x320 3.5" SPI TFT display** and **XPT2046 touch controller**.

---

## ⚡ Key Performance Features

- **40 MHz Hardware SPI Bus**: Maximum hardware-stable SPI clock rate.
- **DMA Double-Buffering**: Overlaps CPU rendering of band $N+1$ with SPI DMA transfer of band $N$.
- **Generalised Dirty-Rectangle Band Skipping**: Divides display into 4×80px bands; skips clean bands completely (0 SPI bytes pushed for static regions).
- **DRAM Tile Atlas Cache**: 12 KB DRAM tile cache eliminates SPI flash `pgm_read_word()` stalls.
- **Achieved Frame Rates**: **18.6+ FPS** on full-screen scrolling scenes; **30–60 FPS** on data-driven UI scenes.

---

## 🚀 Quick Setup & Installation

Clone the repository and run the automated setup script to configure all development dependencies (PlatformIO CLI, Python virtualenv, TTF font tools, build chains, serial USB permissions):

```bash
git clone https://github.com/Paul-PADataAnalytics/base-esp32-ili9488.git
cd base-esp32-ili9488

# Run environment setup script
./scripts/setup_dev_env.sh
```

---

## 🛠️ CLI Development Commands

- **Build Firmware**: `pio run`
- **Flash ESP32**: `pio run --target upload`
- **Serial Monitor**: `python3 monitor.py`
- **Convert TTF Font**: `./venv/bin/python tools/font_converter.py MyFont.ttf 16 > include/fonts/MyFont16pt.h`

---

## 🧩 Touch UI & Choice-Based Game Framework

Include `#include "ui/UIManager.h"` to access the complete touch widget suite:
- **`UILabel`**: Static/dynamic text with alignment options.
- **`UIButton`**: Normal, pressed, disabled, danger states, press callbacks, and long-press (>600ms).
- **`UICheckBox`**: Toggle tickbox with two-way pointer variable binding (`bindTo(&boolVar)`).
- **`UISlider`**: Horizontal value slider with drag support, range limits, and two-way binding (`bindTo(&floatVar)`).
- **`UIOptionSelector`**: Mutually exclusive choice selector (radio group) for choice-based mechanics.
- **`UIFrame`**: Grouping container panel with title bar, border, and touch-drag vertical scrolling.

---

## 📌 Pinout Diagram

| ESP32 Pin | Signal | Target Pin |
|:---:|:---:|:---:|
| **GPIO 18** | SPI SCK | Display SCK / Touch CLK |
| **GPIO 23** | SPI MOSI | Display MOSI / Touch TDIN |
| **GPIO 19** | SPI MISO | Touch TDO |
| **GPIO 15** | Display CS | Display CS |
| **GPIO 2** | Display DC | Display RS/DC |
| **GPIO 4** | Display RST | Display RESET |
| **GPIO 32** | Backlight | Display LED (Active HIGH) |
| **GPIO 33** | Touch CS | Touch CS |
| **GPIO 36** | Touch IRQ | Touch T_IRQ |
