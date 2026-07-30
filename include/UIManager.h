#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <Arduino.h>
#include "GFXContext.h"

/**
 * Toast Notification Banner
 */
struct ToastMessage {
    String message;
    uint16_t color;
    float duration; // seconds
    float timer;
};

/**
 * UIManager - Touch UI Component & Toast Notification Library
 */
class UIManager {
private:
    ToastMessage _activeToast;
    bool _hasToast;

public:
    UIManager() : _hasToast(false) {}

    void showToast(const char* msg, uint16_t color = 0x07FF, float duration = 2.5f) {
        _activeToast.message  = msg;
        _activeToast.color    = color;
        _activeToast.duration = duration;
        _activeToast.timer    = duration;
        _hasToast = true;
    }

    void update(float deltaTime) {
        if (_hasToast) {
            _activeToast.timer -= deltaTime;
            if (_activeToast.timer <= 0.0f) {
                _hasToast = false;
            }
        }
    }

    // --- UI Component Primitives ---

    // Draw Smooth UI Progress Bar
    void drawProgressBar(GFXContext& gfx, int x, int y, int w, int h, float progress, uint16_t barColor, uint16_t bgColor = 0x18C3) {
        progress = constrain(progress, 0.0f, 1.0f);
        gfx.fillRoundRectDirect(x, y, w, h, h / 2, bgColor);
        gfx.drawRoundRectDirect(x, y, w, h, h / 2, 0xFFFF);

        int fillW = (int)((w - 4) * progress);
        if (fillW > 0) {
            gfx.fillRoundRectDirect(x + 2, y + 2, fillW, h - 4, (h - 4) / 2, barColor);
        }
    }

    // Draw Interactive Touch Toggle Switch
    bool drawToggleSwitch(GFXContext& gfx, int x, int y, int w, int h, bool state, const char* label) {
        uint16_t trackColor = state ? gfx.color565(0, 200, 100) : gfx.color565(80, 80, 90);
        gfx.fillRoundRectDirect(x, y, w, h, h / 2, trackColor);
        gfx.drawRoundRectDirect(x, y, w, h, h / 2, 0xFFFF);

        int handleX = state ? (x + w - h + 2) : (x + 2);
        gfx.drawCircleDirect(handleX + (h - 4) / 2, y + h / 2, (h - 4) / 2, 0xFFFF, true);

        if (label) {
            gfx.drawTextDirect(label, x + w + 10, y + (h - 16) / 2, 0xFFFF, 2);
        }

        // Check Touch Hit
        int tx, ty;
        if (gfx.getTouch(&tx, &ty)) {
            if (tx >= x && tx <= x + w && ty >= y && ty <= y + h) {
                return true; // Toggled!
            }
        }
        return false;
    }

    // Render Active Toast Notification
    void renderToast(GFXContext& gfx) {
        if (!_hasToast) return;

        float alpha = _activeToast.timer / _activeToast.duration;
        int toastW = _activeToast.message.length() * 12 + 24;
        int toastH = 34;
        int toastX = (gfx.getWidth() - toastW) / 2;
        int toastY = 40;

        gfx.fillRoundRectDirect(toastX, toastY, toastW, toastH, 8, gfx.color565(20, 30, 50));
        gfx.drawRoundRectDirect(toastX, toastY, toastW, toastH, 8, _activeToast.color);
        gfx.drawTextDirect(_activeToast.message.c_str(), toastX + 12, toastY + 8, _activeToast.color, 2);
    }
};

#endif // UI_MANAGER_H
