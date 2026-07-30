# 🎨 Touch UI & System Component Library (v1.4)

The **`UIManager`**, **`Camera2D`**, **`SoundEngine`**, **`Physics2D`**, and **`SaveSystem`** modules provide a complete suite of user interface and system utilities.

---

## 🎨 Touch UI Component Library (`UIManager.h`)

- **Toast Notifications**: Smooth animated alert banners (`uiManager.showToast("Message", color)`).
- **Progress Bars**: Rounded progress meters (`uiManager.drawProgressBar(...)`).
- **Interactive Toggle Switches**: Touch-responsive ON/OFF sliders (`uiManager.drawToggleSwitch(...)`).

```cpp
#include "UIManager.h"

UIManager uiManager;

void renderUI(GFXContext& gfx) {
    // Show animated toast message
    uiManager.showToast("Level Up!", gfx.color565(0, 255, 120), 2.5f);

    // Draw health bar (75% full)
    uiManager.drawProgressBar(gfx, 20, 280, 150, 20, 0.75f, gfx.color565(255, 0, 0));

    // Render active toasts
    uiManager.renderToast(gfx);
}
```

---

## 🎥 2D Camera Engine (`Camera2D.h`)

- **Smooth Damping (Lerp Tracking)**: Smoothly interpolates position towards target sprite (`camera.setTarget(heroX, heroY)`).
- **Screen Shake FX**: Triggers screen shake on explosions, laser hits, and impacts (`camera.triggerShake(intensity, duration)`).
- **World Bounds**: Enforces boundary bounds (`camera.setWorldBounds(minX, minY, maxX, maxY)`).

---

## 🔊 Retro 8-Bit Audio Synthesizer (`SoundEngine.h`)

- Drives ESP32 LEDC PWM output to piezo speaker or audio jack (GPIO 25/26).
- Pre-programmed retro sound effects:
  - `sound.playCoin()`
  - `sound.playJump()`
  - `sound.playExplosion()`
  - `sound.playLaser()`
  - `sound.playPowerup()`

---

## 💥 2D Physics & Collision Engine (`Physics2D.h`)

- **AABB Box Collisions**: `Physics2D::checkAABB(rect1, rect2)`
- **Circle-to-Circle Collisions**: `Physics2D::checkCircleCollision(circle1, circle2)`
- **Point-in-Rect**: `Physics2D::pointInRect(px, py, rect)`

---

## 💾 Persistent Storage Engine (`SaveSystem.h`)

- Safely persists high scores, settings, and game state in ESP32 Non-Volatile Storage (NVS) across reboots.
- `saveSystem.saveHighScore(score)` & `saveSystem.getHighScore()`.
