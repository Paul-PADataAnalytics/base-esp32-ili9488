# 💤 ESP32 & Display Sleep Manager Module (v1.2)

The **`SleepManager`** module provides developer choice between **Light Sleep** and **Deep Sleep** modes, complete with low-power hardware shutoff, inactivity timers, touch screen wake-up, and Deep Sleep reboot state-restoration assistance.

---

## 🛠️ Sleep Mode Choice: Light Sleep vs Deep Sleep

| Feature | `SleepMode::LIGHT_SLEEP` | `SleepMode::DEEP_SLEEP` |
|---|---|---|
| **Power Consumption** | ~0.8 mA - 2 mA | **Ultra-Low Power (~10 µA)** |
| **RAM State** | Retained in SRAM | Reboots system (Restored via `RTC_DATA_ATTR`) |
| **Wake-up Speed** | Instant resumption after line of code | Fast reboot with `onPostWake()` assistance |
| **Backlight State** | 100% OFF (RTC Hold on GPIO 32) | 100% OFF (RTC Hold on GPIO 32) |
| **Wakeup Source** | Touch `GPIO 36` (PEN) or Timer | Touch `GPIO 36` (PEN) or Timer |

---

## 💻 Developer Code Example with Deep Sleep Assistance

```cpp
#include "SleepManager.h"

// Variable stored in RTC Fast Memory that survives Deep Sleep reboots!
RTC_DATA_ATTR static int rtcWakeupCount = 0;

// Choice: Deep Sleep after 20 seconds of user inactivity
SleepManager sleepManager(20, SleepMode::DEEP_SLEEP);

void setupApp(GFXContext& gfx) {
    // Register Pre-Sleep Callback (Save state to NVS or RTC memory)
    sleepManager.onPreSleep([]() {
        Serial.println("Saving app state before Deep Sleep power down...");
    });

    // Register Post-Wake Callback (Restores app state)
    sleepManager.onPostWake([]() {
        rtcWakeupCount++;
        Serial.printf("Restored state after Deep Sleep! Wakeup Count: %d\n", rtcWakeupCount);
    });

    // ASSISTANT: Automatically checks if system just rebooted from Deep Sleep
    // and triggers onPostWake() callback if true!
    sleepManager.checkAndNotifyDeepSleepWakeup();
}

void renderApp(GFXContext& gfx) {
    int x, y;
    if (gfx.getTouch(&x, &y)) {
        // Touch activity resets inactivity timer
        sleepManager.resetInactivityTimer();
    }

    // Triggers configured sleep mode (Light or Deep)
    if (sleepManager.shouldAutoSleep()) {
        sleepManager.triggerSleep(gfx);
    }
}
```

---

## 🔍 Deep Sleep Assistance API

- `bool SleepManager::wasWokenFromDeepSleep()`: Returns `true` if the ESP32 booted due to a touch or timer wakeup.
- `esp_sleep_wakeup_cause_t SleepManager::getWakeupCause()`: Returns the exact wakeup reason (`ESP_SLEEP_WAKEUP_EXT0` for Touch).
- `void sleepManager.checkAndNotifyDeepSleepWakeup()`: Assistant method to place in `setup()` that automatically invokes `onPostWake()` after a Deep Sleep reboot.
