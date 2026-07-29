# 💤 ESP32 & Display Sleep Manager Module (v1.1)

The **`SleepManager`** module encapsulates low-power Light Sleep, display driver power-down, inactivity timers, and developer lifecycle callbacks.

---

## 🛠️ Key Features for Downstream Developers

1. **Pre-Sleep & Post-Wake Callbacks**:
   - `onPreSleep(callback)`: Executed automatically before the display and ESP32 enter sleep mode. Allows downstream applications to save game state, close open network connections, or write data to flash.
   - `onPostWake(callback)`: Executed automatically after the display wakes up and the backlight turns back on. Allows applications to reload state or resume audio/animations.

2. **Automated Inactivity Timeout**:
   - Automatically tracks user touch input. If no touch activity occurs for $N$ seconds, the unit automatically powers down hardware and enters low-power Light Sleep.

3. **Touch & Timer Wake-up**:
   - **Touch Wakeup**: Wakes up instantly when the user touches the screen (`GPIO 36` / `PEN` interrupt).
   - **Timer Wakeup**: Optional auto-wake after $S$ seconds.

---

## 💻 Developer Code Example

```cpp
#include "SleepManager.h"

SleepManager sleepManager(30); // 30-second auto-sleep timeout

void setupApp(GFXContext& gfx) {
    // Register Pre-Sleep Callback (Save state)
    sleepManager.onPreSleep([]() {
        Serial.println("Saving game state before display turns off...");
    });

    // Register Post-Wake Callback (Resume state)
    sleepManager.onPostWake([]() {
        Serial.println("Resuming game state after display wake up!");
    });
}

void renderApp(GFXContext& gfx) {
    int x, y;
    if (gfx.getTouch(&x, &y)) {
        // Touch activity resets the inactivity sleep timer automatically
        sleepManager.resetInactivityTimer();
    }

    // Trigger auto-sleep when inactivity timeout elapses
    if (sleepManager.shouldAutoSleep()) {
        sleepManager.enterLightSleep(gfx);
    }
}
```

---

## ⚡ Hardware Power-Down Sequence

During `enterLightSleep()`:
1. Calls developer `onPreSleep()` callback.
2. Turns OFF LED Backlight (`GPIO 32` LOW).
3. Sends `SLPIN` (0x10) command to ILI9488 display driver.
4. Configures ESP32 `ext0` / GPIO wakeup on `GPIO 36` (`PEN` pin).
5. Enters ESP32 Light Sleep mode.

During Wakeup:
1. Sends `SLPOUT` (0x11) command to ILI9488 display driver.
2. Turns ON LED Backlight (`GPIO 32` HIGH).
3. Resets inactivity timer.
4. Calls developer `onPostWake()` callback.
