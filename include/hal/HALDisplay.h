#ifndef HAL_DISPLAY_H
#define HAL_DISPLAY_H

#include <cstdint>

#if defined(PLATFORM_LINUX) || !defined(ARDUINO)
#include "../Arduino_Linux.h"
#else
#include <Arduino.h>
#endif

/**
 * HALDisplay — Abstract Hardware Abstraction Layer Interface for Display & Touch
 *
 * Decouples engine and application logic from physical LCD drivers and operating systems.
 * Provides concrete implementations for:
 *   - ESP32 Hardware HAL (LovyanGFX ILI9488 + XPT2046 40 MHz SPI DMA)
 *   - Linux Desktop Simulator HAL (Native SDL2 Window + Mouse Touch Emulation)
 */
class HALDisplay {
public:
    virtual ~HALDisplay() = default;

    /** Initialize display hardware or desktop simulator window. */
    virtual bool init() = 0;

    /** Set display orientation (0: Portrait, 1: Landscape 480x320). */
    virtual void setRotation(uint8_t rotation) = 0;

    /** Fill display screen with RGB565 color. */
    virtual void fillScreen(uint16_t color) = 0;

    /** Get active touch or mouse coordinates in 480x320 canvas space. */
    virtual bool getTouch(int* x, int* y) = 0;

    /** Check if touch screen or mouse button is currently pressed. */
    virtual bool isTouched() = 0;

    /** Poll system/window events (SDL events on desktop, no-op on ESP32). */
    virtual void pollEvents() = 0;

    /** Check if simulator window or application loop is running. */
    virtual bool isRunning() const = 0;

    /** Convert RGB888 components to 16-bit RGB565. */
    virtual uint16_t color565(uint8_t r, uint8_t g, uint8_t b) = 0;

    /** Turn backlight on/off. */
    virtual void setBacklight(bool on) = 0;

    /** Put display to sleep / wake up. */
    virtual void sleep() = 0;
    virtual void wakeup() = 0;
};

#endif // HAL_DISPLAY_H
