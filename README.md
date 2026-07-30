# 🚀 ESP32 WROOM + ILI9488 3.5" TFT LCD + XPT2046 Touch HAL Framework (v1.4)

A high-performance, zero-flicker, hardware-abstracted graphics, retro game engine, touch UI, audio, physics, and power-management framework for ESP32 and ILI9488 3.5" TFT displays with XPT2046 touch controllers.

[![Version](https://img.shields.io/badge/version-1.4.0-blue.svg)](https://github.com/Paul-PADataAnalytics/base-esp32-ili9488)
[![Platform](https://img.shields.io/badge/platform-ESP32-orange.svg)](https://platformio.org/)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

---

## 📌 Features

- **🏛️ Hardware Abstraction Layer (HAL)**: Hardware control (SPI DMA, LCD registers, GPIO pins, backlight, touch driver, and power management) is completely isolated from user application code.
- **🎥 2D Camera Engine & Screen Shake (v1.4)**: Smooth target tracking (`Camera2D`), lerp damping, world bounds, and screen shake FX for explosions.
- **🔊 Retro 8-Bit Audio Synthesizer (v1.4)**: ESP32 LEDC PWM tone synthesizer (`SoundEngine`) with pre-programmed retro SFX (`playCoin`, `playJump`, `playExplosion`, `playLaser`, `playPowerup`).
- **💥 2D Physics & Collisions (v1.4)**: Axis-Aligned Bounding Box (AABB) and Circle-to-Circle collision detection (`Physics2D`).
- **💾 NVS Persistent Storage (v1.4)**: High score, settings, and game state persistence across reboots (`SaveSystem`).
- **🎨 Touch UI Component Library (v1.4)**: Animated toast notifications, rounded progress bars, interactive touch toggle switches (`UIManager`). See **[gui.md](gui.md)**.
- **🎮 Retro Game Engine Subsystem**: Multi-Layer Compositing Engine (`LayerManager`), Background & Foreground Layer classification, independent layer matrix transforms (scroll, scale, rotation), `TileSet` atlases, 2D `TileMap` grid levels, 2D Affine `RetroSprite` & `AnimatedSprite` (translation, rotation, scale, skewing, custom pivot), `ParticleEngine` explosions, and `VirtualGamepad` touch controls. See **[retro.md](retro.md)**.
- **💤 Sleep & Power Manager Choice**: Choice between **Light Sleep** and **Deep Sleep** (~10 µA ultra-low power), RTC GPIO 32 backlight shutoff, developer pre-sleep & post-wake callbacks, and Deep Sleep reboot state-restoration assistance. See **[sleep.md](sleep.md)**.
- **⚡ 0% Flicker Double-Buffering**: Overlapping 3D projections, sprites, tilemaps, and bouncing objects are rendered inside an off-screen sprite buffer before DMA flushing.
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
| **Audio SFX**   | **BUZZER**  | **GPIO 25** | Audio Tone Output |

For full pinout details, see **[pinout.md](pinout.md)**.

---

## 📜 Version History

- **v1.4.0**:
  - Added **2D Camera Engine & Screen Shake** (`Camera2D.h`).
  - Added **Retro 8-Bit Sound FX Synthesizer** (`SoundEngine.h`).
  - Added **2D Physics Collision Engine** (`Physics2D.h`).
  - Added **NVS Persistent Storage** (`SaveSystem.h`).
  - Added **Touch UI Component Library** (`UIManager.h`).
  - See [gui.md](gui.md) for full UI & system documentation.
- **v1.3.0**:
  - Added **Retro Game Engine Subsystem** (`TileSet.h`, `TileMap.h`, `LayerManager.h`, `RetroSprite.h`, `ParticleEngine.h`, `VirtualGamepad.h`).
- **v1.2.0**:
  - Added **Image Loading & Display Primitives** (`pushImageDirect`, `pushImageBuffer`, `pushImageTransparent`).
  - Added **Deep Sleep Choice** (`SleepMode::DEEP_SLEEP` vs `SleepMode::LIGHT_SLEEP`).
- **v1.1.0**:
  - Added **Sleep & Power Manager Module** (`SleepManager.h`).
- **v1.0.0**: 
  - Complete Hardware Abstraction Layer (HAL) architecture (`BaseApp`, `GFXContext`, `AppEngine`, `DisplayDriver`).
