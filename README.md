# ESP32 WROOM + ILI9488 LCD Animation Project

A high-performance hardware animation demo running on an **ESP32-WROOM-32** microcontroller and an **ILI9488 3.5" TFT SPI Display** (480x320 resolution).

This project features:
- **Zero-flicker double-buffered sprite rendering** using SPI DMA.
- **Real-time 3D Wireframe & Point Cloud rotation engine**.
- **Dynamic 3D Particle Starfield & Cyberpunk Neon Grid background**.
- **On-screen Real-time FPS Performance Counter**.

---

## 📌 Pinout & Wiring Diagram

The diagram below illustrates the exact pin connections between the ESP32 WROOM development board and the ILI9488 3.5" SPI LCD module.

![ESP32 WROOM to ILI9488 Pinout Diagram](/home/paul/.gemini/antigravity/brain/e43a7ee9-1984-41ea-a74e-c917ab34426c/esp32_ili9488_pinout_1785337187284.png)

### Pin Connection Table

| ESP32 Pin | ILI9488 LCD Pin | Signal Description | Note |
|-----------|------------------|-------------------|------|
| **5V / VIN** | **VCC** | Power Supply (5V or 3.3V depending on module regulator) | Requires 5V if module has 3.3V LDO |
| **GND** | **GND** | Ground | Common Ground |
| **GPIO 5** | **CS** | Chip Select | SPI SS |
| **GPIO 4** | **RESET / RST** | Hardware Reset | Display Reset |
| **GPIO 2** | **DC / RS** | Data / Command Select | Control Pin |
| **GPIO 23**| **SDI / MOSI** | SPI Data Input | Master Out Slave In |
| **GPIO 18**| **SCK / CLK** | SPI Clock | Serial Clock |
| **GPIO 19**| **SDO / MISO** | SPI Data Output | Master In Slave Out (Optional) |
| **3.3V** | **LED / BL** | Backlight Power | Can connect to GPIO for PWM dimming |

---

## 🚀 How to Build & Flash

### Option A: Using PlatformIO (Recommended)
1. Clone or open this repository in VS Code with the PlatformIO extension installed.
2. Connect your ESP32 WROOM via USB.
3. Run:
   ```bash
   pio run --target upload
   ```
4. Open Serial Monitor at `115200` baud rate to check logs.

### Option B: Using Arduino IDE
1. Open Arduino IDE and install the **LovyanGFX** or **TFT_eSPI** library via the Library Manager.
2. Select Board: `ESP32 Dev Module`.
3. If using `TFT_eSPI`, copy the contents of `include/User_Setup.h` into `Arduino/libraries/TFT_eSPI/User_Setup.h`.
4. Open `src/main.cpp` (rename to `.ino` if desired), compile, and upload!

---

## ⚡ Performance Optimization Tips
- **SPI Frequency**: ILI9488 displays reliably support 27 MHz to 40 MHz SPI write speed.
- **Double Buffering / Sprites**: Drawing directly to the LCD over SPI causes tearing and flicker. Utilizing `LGFX_Sprite` or `TFT_eSprite` allocates a framebuffer in ESP32 RAM and pushes updates via SPI DMA for buttery smooth 60 FPS performance.
