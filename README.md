# 🚀 ESP32 WROOM + ILI9488 3.5" TFT LCD + XPT2046 Touch HAL Framework (v1.2)

A high-performance, zero-flicker, hardware-abstracted graphics, image-loading, touch, and power-management framework for ESP32 and ILI9488 3.5" TFT displays with XPT2046 touch controllers.

[![Version](https://img.shields.io/badge/version-1.2.0-blue.svg)](https://github.com/Paul-PADataAnalytics/base-esp32-ili9488)
[![Platform](https://img.shields.io/badge/platform-ESP32-orange.svg)](https://platformio.org/)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

---

## 📌 Features

- **🏛️ Hardware Abstraction Layer (HAL)**: Hardware control (SPI DMA, LCD registers, GPIO pins, backlight, touch driver, and power management) is completely isolated from user application code.
- **🖼️ Image & Graphics Primitives (v1.2)**: 16-bit RGB565 bitmap array loading, transparent color keying (`pushImageTransparent`), color blending (`blendColor`), 16-bit color gradients, rounded card rects, and UI button components.
- **💤 Sleep & Power Manager Choice (v1.2)**: Choice between **Light Sleep** and **Deep Sleep** (~10 µA ultra-low power), RTC GPIO 32 backlight shutoff, developer pre-sleep & post-wake callbacks, and Deep Sleep reboot state-restoration assistance. See **[sleep.md](sleep.md)**.
- **⚡ 0% Flicker Double-Buffering**: Overlapping 3D projections, images, and bouncing objects are rendered inside an off-screen sprite buffer before DMA flushing.
- **👆 Integrated Touch Screen Support**: Touch input (XPT2046) is handled transparently through the `GFXContext` API.
- **📌 Dedicated Pinout Guide**: See **[pinout.md](pinout.md)** for full wiring tables and schematic diagrams.

---

## 🖼️ Image Loading & Rendering API

```cpp
#include "SampleIcons.h" // 16-bit RGB565 Bitmap Array

void renderApp(GFXContext& gfx) {
    // 1. Draw direct or double-buffered bitmap image
    gfx.pushImageBuffer(x, y, width, height, bitmapData);

    // 2. Draw transparent image ignoring key color (e.g. 0x0000 black)
    gfx.pushImageTransparent(x, y, 16, 16, ICON_POWER_16x16, 0x0000);

    // 3. Draw smooth 16-bit color gradient
    gfx.drawGradientRectDirect(0, 0, 480, 32, startColor, endColor, true);
}
```

---

## 💤 Sleep Manager & Deep Sleep Assistance

```cpp
#include "SleepManager.h"

// Variable stored in RTC memory surviving Deep Sleep reboots
RTC_DATA_ATTR static int rtcWakeupCount = 0;

// Choice: DEEP_SLEEP mode after 15 seconds of inactivity
SleepManager sleepManager(15, SleepMode::DEEP_SLEEP);

void setupApp(GFXContext& gfx) {
    // Pre-Sleep Callback: Save state before hardware powers down
    sleepManager.onPreSleep([]() {
        Serial.println("Saving app state before Deep Sleep...");
    });

    // Post-Wake Callback: Restore state after touch wake-up
    sleepManager.onPostWake([]() {
        rtcWakeupCount++;
    });

    // Assistant check for Deep Sleep reboot wakeup
    sleepManager.checkAndNotifyDeepSleepWakeup();
}
```

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

## 📜 Version History

- **v1.2.0**:
  - Added **Image Loading & Display Primitives** (`pushImageDirect`, `pushImageBuffer`, `pushImageTransparent`).
  - Added **Advanced Graphics Primitives** (`blendColor`, `drawGradientRectDirect`, `fillRoundRectDirect`, `drawButtonDirect`).
  - Added **Deep Sleep Choice** (`SleepMode::DEEP_SLEEP` vs `SleepMode::LIGHT_SLEEP`).
  - Added **Deep Sleep Reboot Assistance** (`wasWokenFromDeepSleep()`, `checkAndNotifyDeepSleepWakeup()`, `RTC_DATA_ATTR` support).
- **v1.1.0**:
  - Added **Sleep & Power Manager Module** (`SleepManager.h`).
  - Added developer `onPreSleep()` and `onPostWake()` lifecycle callbacks.
  - Automated inactivity sleep timer and Touch (`GPIO 36`) wake-up support.
- **v1.0.0**: 
  - Complete Hardware Abstraction Layer (HAL) architecture (`BaseApp`, `GFXContext`, `AppEngine`, `DisplayDriver`).
  - XPT2046 touch controller integration on shared VSPI bus (`GPIO 18`, `23`, `19`, `33`, `36`).
  - Write-Only LCD SPI mode (`pin_miso = -1`) preventing breadboard MISO contention.
  - Zero-flicker double-buffered 3D animation engine at 23+ FPS.
