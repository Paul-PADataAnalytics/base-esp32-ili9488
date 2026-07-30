#ifndef SLEEP_MANAGER_H
#define SLEEP_MANAGER_H

#include <Arduino.h>
#include <esp_sleep.h>
#include <driver/gpio.h>
#include <functional>
#include "GFXContext.h"

/**
 * Supported Hardware Sleep Modes
 */
enum class SleepMode {
    LIGHT_SLEEP, // Fast resume, preserves RAM state (~1 mA)
    DEEP_SLEEP   // Ultra-low power, reboots on wakeup with state assistance (~10 uA)
};

/**
 * SleepManager - ESP32 & Display Power/Sleep Management Module (v1.2)
 * 
 * Provides developer choice between Light Sleep and Deep Sleep modes,
 * automated inactivity timers, RTC GPIO pin holding, and state-restoration
 * assistance for Deep Sleep reboots.
 */
class SleepManager {
public:
    using SleepCallback = std::function<void()>;

private:
    uint32_t      _inactivityTimeoutMs;
    unsigned long _lastActivityTime;
    bool          _autoSleepEnabled;
    bool          _isSleeping;
    SleepMode     _chosenSleepMode;

    SleepCallback _preSleepCallback;
    SleepCallback _postWakeCallback;

public:
    SleepManager(uint32_t inactivityTimeoutSeconds = 0, SleepMode mode = SleepMode::LIGHT_SLEEP)
        : _inactivityTimeoutMs(inactivityTimeoutSeconds * 1000),
          _lastActivityTime(0),
          _autoSleepEnabled(inactivityTimeoutSeconds > 0),
          _isSleeping(false),
          _chosenSleepMode(mode),
          _preSleepCallback(nullptr),
          _postWakeCallback(nullptr) {}

    // Developer Lifecycle Callbacks
    void onPreSleep(SleepCallback cb) { _preSleepCallback = cb; }
    void onPostWake(SleepCallback cb) { _postWakeCallback = cb; }

    // Sleep Mode Selection API
    void setSleepMode(SleepMode mode) { _chosenSleepMode = mode; }
    SleepMode getSleepMode() const { return _chosenSleepMode; }

    // Inactivity Timer Control
    void resetInactivityTimer() {
        _lastActivityTime = millis();
    }

    void setAutoSleepTimeout(uint32_t seconds) {
        _inactivityTimeoutMs = seconds * 1000;
        _autoSleepEnabled    = (seconds > 0);
        resetInactivityTimer();
    }

    void disableAutoSleep() {
        _autoSleepEnabled = false;
    }

    void enableAutoSleep(uint32_t seconds = 20) {
        setAutoSleepTimeout(seconds);
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

    // --- Deep Sleep Developer Assistance Helpers ---

    /**
     * Checks if the current boot was triggered by waking up from Deep Sleep.
     */
    static bool wasWokenFromDeepSleep() {
        esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
        return (cause != ESP_SLEEP_WAKEUP_UNDEFINED && cause != ESP_SLEEP_WAKEUP_GPIO);
    }

    /**
     * Returns the exact ESP-IDF wakeup reason (EXT0 Touch, Timer, etc.)
     */
    static esp_sleep_wakeup_cause_t getWakeupCause() {
        return esp_sleep_get_wakeup_cause();
    }

    /**
     * Assistant helper to be called in app setup() after Deep Sleep reboot.
     * Automatically triggers the registered onPostWake() callback if waking from Deep Sleep.
     */
    void checkAndNotifyDeepSleepWakeup() {
        if (wasWokenFromDeepSleep() && _postWakeCallback) {
            Serial.println("[SLEEP MANAGER ASSISTANT] Detected Deep Sleep reboot wakeup! Running post-wake callback.");
            _postWakeCallback();
        }
    }

    // --- Execute Sleep Mode (Light or Deep based on choice) ---
    void triggerSleep(GFXContext& gfx, uint32_t sleepSeconds = 0) {
        if (_chosenSleepMode == SleepMode::DEEP_SLEEP) {
            enterDeepSleep(gfx, sleepSeconds);
        } else {
            enterLightSleep(gfx, sleepSeconds);
        }
    }

    // Put ESP32 into Low-Power Light Sleep
    void enterLightSleep(GFXContext& gfx, uint32_t sleepSeconds = 0) {
        if (_isSleeping) return;

        Serial.println("\n[SLEEP MANAGER] Executing Pre-Light-Sleep Routines...");

        if (_preSleepCallback) {
            _preSleepCallback();
        }

        // Turn OFF Backlight & Hold GPIO 32 LOW during sleep
        gfx.turnOffBacklight();
        gpio_hold_en((gpio_num_t)32);
        gpio_deep_sleep_hold_en();

        gfx.sleepDisplay();
        _isSleeping = true;
        Serial.println("[SLEEP MANAGER] Backlight OFF (GPIO 32 LOW Hold). ESP32 entering Light Sleep.");
        Serial.flush();

        if (sleepSeconds > 0) {
            esp_sleep_enable_timer_wakeup((uint64_t)sleepSeconds * 1000000ULL);
        }
        
        // Touch Screen Wakeup (GPIO 36 / PEN Interrupt active LOW)
        gpio_wakeup_enable((gpio_num_t)36, GPIO_INTR_LOW_LEVEL);
        esp_sleep_enable_gpio_wakeup();

        esp_light_sleep_start();

        // WAKE UP
        _isSleeping = false;
        resetInactivityTimer();

        Serial.println("\n[SLEEP MANAGER] Waking up from Light Sleep...");
        gpio_hold_dis((gpio_num_t)32);
        gfx.wakeDisplay();
        gfx.turnOnBacklight();

        if (_postWakeCallback) {
            _postWakeCallback();
        }
    }

    // Put ESP32 into Ultra-Low Power Deep Sleep (Reboots on wakeup)
    void enterDeepSleep(GFXContext& gfx, uint32_t sleepSeconds = 0) {
        Serial.println("\n[SLEEP MANAGER] Executing Pre-Deep-Sleep Routines...");

        if (_preSleepCallback) {
            _preSleepCallback();
        }

        // 1. Turn OFF Backlight & Deselect Touch CS
        gfx.turnOffBacklight();
        digitalWrite(33, HIGH); // Deselect Touch CS

        // Apply RTC GPIO hold for GPIO 32 (Backlight LOW) and GPIO 33 (TCS HIGH)
        gpio_hold_en((gpio_num_t)32);
        gpio_hold_en((gpio_num_t)33);
        gpio_deep_sleep_hold_en();

        // 2. Sleep Display Controller
        gfx.sleepDisplay();

        Serial.println("[SLEEP MANAGER] Display powered down. ESP32 entering Ultra-Low-Power Deep Sleep.");
        Serial.flush();

        // 3. Configure Wakeup Sources for Deep Sleep
        if (sleepSeconds > 0) {
            esp_sleep_enable_timer_wakeup((uint64_t)sleepSeconds * 1000000ULL);
        }

        // Configure EXT0 Wakeup on GPIO 36 (Touch PEN Interrupt active LOW = 0)
        esp_sleep_enable_ext0_wakeup(GPIO_NUM_36, 0);

        // 4. Enter Deep Sleep (reboots hardware on touch or timer!)
        esp_deep_sleep_start();
    }
};

#endif // SLEEP_MANAGER_H
