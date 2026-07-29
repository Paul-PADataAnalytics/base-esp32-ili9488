#ifndef SLEEP_MANAGER_H
#define SLEEP_MANAGER_H

#include <Arduino.h>
#include <esp_sleep.h>
#include <driver/gpio.h>
#include <functional>
#include "GFXContext.h"

/**
 * SleepManager - ESP32 & Display Power/Sleep Management Module
 * 
 * Configures GPIO 32 RTC pin hold to guarantee the display backlight turns off
 * 100% completely during Light Sleep when LED is connected to GPIO 32.
 */
class SleepManager {
public:
    using SleepCallback = std::function<void()>;

private:
    uint32_t      _inactivityTimeoutMs;
    unsigned long _lastActivityTime;
    bool          _autoSleepEnabled;
    bool          _isSleeping;

    SleepCallback _preSleepCallback;
    SleepCallback _postWakeCallback;

public:
    SleepManager(uint32_t inactivityTimeoutSeconds = 20)
        : _inactivityTimeoutMs(inactivityTimeoutSeconds * 1000),
          _lastActivityTime(0),
          _autoSleepEnabled(true),
          _isSleeping(false),
          _preSleepCallback(nullptr),
          _postWakeCallback(nullptr) {}

    // Developer Callbacks
    void onPreSleep(SleepCallback cb) { _preSleepCallback = cb; }
    void onPostWake(SleepCallback cb) { _postWakeCallback = cb; }

    void resetInactivityTimer() {
        _lastActivityTime = millis();
    }

    void setAutoSleepTimeout(uint32_t seconds) {
        _inactivityTimeoutMs = seconds * 1000;
        _autoSleepEnabled    = (seconds > 0);
        resetInactivityTimer();
    }

    bool isAutoSleepEnabled() const { return _autoSleepEnabled; }
    bool isSleeping() const { return _isSleeping; }

    uint32_t getInactivitySecondsRemaining() const {
        if (!_autoSleepEnabled || _isSleeping) return 0;
        unsigned long elapsed = millis() - _lastActivityTime;
        if (elapsed >= _inactivityTimeoutMs) return 0;
        return (_inactivityTimeoutMs - elapsed) / 1000;
    }

    bool shouldAutoSleep() const {
        if (!_autoSleepEnabled || _isSleeping) return false;
        return (millis() - _lastActivityTime >= _inactivityTimeoutMs);
    }

    // Put ESP32, Display & Touch into Low-Power Light Sleep
    void enterLightSleep(GFXContext& gfx, uint32_t sleepSeconds = 0) {
        if (_isSleeping) return;

        Serial.println("\n[SLEEP MANAGER] Executing Pre-Sleep Routines...");

        // 1. Invoke developer pre-sleep callback
        if (_preSleepCallback) {
            _preSleepCallback();
        }

        // 2. Turn OFF Backlight & Hold GPIO 32 LOW during sleep
        gfx.turnOffBacklight();
        gpio_hold_en((gpio_num_t)32);
        gpio_deep_sleep_hold_en();

        // 3. Put display controller to sleep
        gfx.sleepDisplay();

        _isSleeping = true;
        Serial.println("[SLEEP MANAGER] Backlight OFF (GPIO 32 LOW Hold). ESP32 entering Light Sleep.");
        Serial.flush();

        // 4. Configure Wakeup Sources
        if (sleepSeconds > 0) {
            esp_sleep_enable_timer_wakeup((uint64_t)sleepSeconds * 1000000ULL);
        }
        
        // Touch Screen Wakeup (GPIO 36 / PEN Interrupt active LOW)
        gpio_wakeup_enable((gpio_num_t)36, GPIO_INTR_LOW_LEVEL);
        esp_sleep_enable_gpio_wakeup();

        // 5. Enter ESP32 Light Sleep
        esp_light_sleep_start();

        // --- WAKE UP ROUTINE ---
        _isSleeping = false;
        resetInactivityTimer();

        Serial.println("\n[SLEEP MANAGER] Waking up hardware...");

        // 6. Release GPIO hold & turn ON Backlight
        gpio_hold_dis((gpio_num_t)32);
        gfx.wakeDisplay();
        gfx.turnOnBacklight();

        // 7. Invoke developer post-wake callback
        if (_postWakeCallback) {
            _postWakeCallback();
        }
        
        Serial.println("[SLEEP MANAGER] Post-wake routines complete.");
    }
};

#endif // SLEEP_MANAGER_H
