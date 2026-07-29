# 🚀 ESP32 WROOM + ILI9488 3.5" TFT LCD + XPT2046 Touch HAL Framework (v1.3)

A high-performance, zero-flicker, hardware-abstracted graphics, retro game engine, touch, and power-management framework for ESP32 and ILI9488 3.5" TFT displays with XPT2046 touch controllers.

[![Version](https://img.shields.io/badge/version-1.3.0-blue.svg)](https://github.com/Paul-PADataAnalytics/base-esp32-ili9488)
[![Platform](https://img.shields.io/badge/platform-ESP32-orange.svg)](https://platformio.org/)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

---

## 📌 Features

- **🏛️ Hardware Abstraction Layer (HAL)**: Hardware control (SPI DMA, LCD registers, GPIO pins, backlight, touch driver, and power management) is completely isolated from user application code.
- **🎮 Retro Game Engine Subsystem (v1.3)**: Multi-Layer Compositing Engine (`LayerManager`), Background & Foreground Layer classification, independent layer matrix transforms (scroll, scale, rotation), `TileSet` atlases, 2D `TileMap` grid levels, 2D Affine `RetroSprite` & `AnimatedSprite` (translation, rotation, scale, skewing, custom pivot), `ParticleEngine` explosions, and `VirtualGamepad` touch controls. See **[retro.md](retro.md)**.
- **🖼️ Image & Graphics Primitives**: 16-bit RGB565 bitmap array loading, transparent color keying (`pushImageTransparent`), color blending (`blendColor`), 16-bit color gradients, rounded card rects, and UI button components.
- **💤 Sleep & Power Manager Choice**: Choice between **Light Sleep** and **Deep Sleep** (~10 µA ultra-low power), RTC GPIO 32 backlight shutoff, developer pre-sleep & post-wake callbacks, and Deep Sleep reboot state-restoration assistance. See **[sleep.md](sleep.md)**.
- **⚡ 0% Flicker Double-Buffering**: Overlapping 3D projections, sprites, tilemaps, and bouncing objects are rendered inside an off-screen sprite buffer before DMA flushing.
- **👆 Integrated Touch Screen Support**: Touch input (XPT2046) is handled transparently through the `GFXContext` API.
- **📌 Dedicated Pinout Guide**: See **[pinout.md](pinout.md)** for full wiring tables and schematic diagrams.

---

## 🎮 Retro Game Engine & Layer Pipeline (v1.3)

```cpp
#include "LayerManager.h"
#include "RetroSprite.h"
#include "TileMap.h"

// Define Hero Sprite with Translation, Rotation, Scale, and Skewing
RetroSprite hero(ICON_POWER_16x16, 16, 16, 0x0000);

void setupApp(GFXContext& gfx) {
    hero.setPosition(180, 120);
    hero.setScale(2.0f, 2.0f);

    // Add Background Layer (renders below everything else)
    layerManager.addLayer("Background", LayerRole::BACKGROUND, 0, [](GFXContext& gfx, LGFX_Sprite* buf, Layer& layer) {
        // Rotating background starfield
    });

    // Add Foreground Layer (renders above world & entities for tree canopy occlusion!)
    layerManager.addLayer("Foreground", LayerRole::FOREGROUND, 0, [](GFXContext& gfx, LGFX_Sprite* buf, Layer& layer) {
        // Render tree tops over sprites
    });
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

- **v1.3.0**:
  - Added **Retro Game Engine Subsystem** (`TileSet.h`, `TileMap.h`, `LayerManager.h`, `RetroSprite.h`, `ParticleEngine.h`, `VirtualGamepad.h`).
  - Added **Background & Foreground Layer Classification** (`LayerRole::BACKGROUND`, `LayerRole::FOREGROUND`).
  - Added **Independent Layer Matrix Transforms** (rotation, translation, scale per layer).
  - Added **2D Affine Matrix Sprite Engine** (translation, rotation, scale, skewing, custom pivot, transparent keying).
  - Added **Multi-Frame Animated Sprite Engine** (`AnimatedSprite`).
  - Added **2D Particle System** (`ParticleEngine`) & **Touch Virtual Gamepad** (`VirtualGamepad`).
  - See [retro.md](retro.md) for full retro engine documentation.
- **v1.2.0**:
  - Added **Image Loading & Display Primitives** (`pushImageDirect`, `pushImageBuffer`, `pushImageTransparent`).
  - Added **Deep Sleep Choice** (`SleepMode::DEEP_SLEEP` vs `SleepMode::LIGHT_SLEEP`).
- **v1.1.0**:
  - Added **Sleep & Power Manager Module** (`SleepManager.h`).
- **v1.0.0**: 
  - Complete Hardware Abstraction Layer (HAL) architecture (`BaseApp`, `GFXContext`, `AppEngine`, `DisplayDriver`).
