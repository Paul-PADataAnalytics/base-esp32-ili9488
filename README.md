# 🚀 ESP32 WROOM + ILI9488 3.5" TFT LCD + XPT2046 Touch HAL Framework (v1.1)

A high-performance, zero-flicker, hardware-abstracted graphics, touch, and power-management framework for ESP32 and ILI9488 3.5" TFT displays with XPT2046 touch controllers.

[![Version](https://img.shields.io/badge/version-1.1.0-blue.svg)](https://github.com/Paul-PADataAnalytics/base-esp32-ili9488)
[![Platform](https://img.shields.io/badge/platform-ESP32-orange.svg)](https://platformio.org/)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

---

## 📌 Features

- **🏛️ Hardware Abstraction Layer (HAL)**: Hardware control (SPI DMA, LCD registers, GPIO pins, backlight, touch driver, and power management) is completely isolated from user application code.
- **💤 Sleep & Power Manager Module (v1.1)**: Built-in inactivity sleep timers, manual sleep triggers, developer pre-sleep & post-wake callbacks (`onPreSleep`, `onPostWake`), and touch screen wake-up (`GPIO 36`). See **[sleep.md](sleep.md)**.
- **⚡ 0% Flicker Double-Buffering**: Overlapping 3D projections and bouncing objects are rendered inside an off-screen sprite buffer before DMA flushing.
- **👆 Integrated Touch Screen Support**: Touch input (XPT2046) is handled transparently through the `GFXContext` API.
- **📌 Dedicated Pinout Guide**: See **[pinout.md](pinout.md)** for full wiring tables and schematic diagrams.

---

## 🔌 Hardware Connections Summary

| Pin Type | Signal Label | ESP32 GPIO | Description |
|---|---|---|---|
| **LCD Display** | **VCC** | **VIN (5V)** | 5V Main Power |
| **LCD Display** | **GND** | **GND** | Ground |
| **LCD Display** | **CS** | **GPIO 5** | LCD Chip Select |
| **LCD Display** | **RESET** | **GPIO 4** | LCD Reset |
| **LCD Display** | **DC / RS** | **GPIO 27** | Data / Command Select |
| **LCD Display** | **LED** | **GPIO 32** | Backlight Power |
| **Shared SPI** | **SCK / TCK** | **GPIO 18** | Shared SPI Clock |
| **Shared SPI** | **MOSI / TDI**| **GPIO 23** | Shared SPI Data In |
| **Shared SPI** | **MISO / TDO**| **GPIO 19** | Shared SPI Data Out |
| **Touch Screen**| **TCS** | **GPIO 33** | Touch Chip Select |
| **Touch Screen**| **PEN** | **GPIO 36** | Touch Interrupt |

For full pinout details, see **[pinout.md](pinout.md)**.

---

## 💤 Sleep Manager & Developer Callbacks (v1.1)

```cpp
#include "SleepManager.h"

SleepManager sleepManager(20); // 20-second inactivity timeout

void setupApp(GFXContext& gfx) {
    // Pre-Sleep Callback: Save state before hardware powers down
    sleepManager.onPreSleep([]() {
        Serial.println("Saving app state before sleep...");
    });

    // Post-Wake Callback: Restore state after display powers on
    sleepManager.onPostWake([]() {
        Serial.println("Restoring app state after wake!");
    });
}
```

---

## 🚀 How to Build a New Application

Inherit from `BaseApp` and implement your custom application:

```cpp
#include "BaseApp.h"
#include "GFXContext.h"

class MyTouchApp : public BaseApp {
public:
    void setup(GFXContext& gfx) override {}
    void update(float deltaTime) override {}

    void render(GFXContext& gfx) override {
        int x, y;
        if (gfx.getTouch(&x, &y)) {
            // Draw interactive touch ripple
            gfx.drawCircleDirect(x, y, 15, gfx.color565(0, 255, 255), true);
        }
        gfx.pushBuffer();
    }
};
```

---

## 🛠️ Building & Flashing

Built using PlatformIO:

```bash
# Compile firmware
pio run

# Flash to connected ESP32
pio run --target upload
```

---

## 📜 Version History

- **v1.1.0**:
  - Added **Sleep & Power Manager Module** (`SleepManager.h`).
  - Added developer `onPreSleep()` and `onPostWake()` lifecycle callbacks.
  - Automated inactivity sleep timer and Touch (`GPIO 36`) wake-up support.
  - See [sleep.md](sleep.md) for full sleep module documentation.
- **v1.0.0**: 
  - Complete Hardware Abstraction Layer (HAL) architecture (`BaseApp`, `GFXContext`, `AppEngine`, `DisplayDriver`).
  - XPT2046 touch controller integration on shared VSPI bus (`GPIO 18`, `23`, `19`, `33`, `36`).
  - Write-Only LCD SPI mode (`pin_miso = -1`) preventing breadboard MISO contention.
  - Zero-flicker double-buffered 3D animation engine at 23+ FPS.
