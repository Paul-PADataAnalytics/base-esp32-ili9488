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
    float duration;
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

    // Render Active Toast Notification onto full 480x320 screen
    void renderToast(GFXContext& gfx) {
        if (!_hasToast) return;

        int toastW = _activeToast.message.length() * 12 + 24;
        int toastH = 34;
        int toastX = (gfx.getWidth() - toastW) / 2;
        int toastY = 40;

        gfx.fillRoundRectDirect(toastX, toastY, toastW, toastH, 8, gfx.color565(20, 30, 50));
        gfx.drawRoundRectDirect(toastX, toastY, toastW, toastH, 8, _activeToast.color);
        gfx.drawTextDirect(_activeToast.message.c_str(), toastX + 12, toastY + 8, _activeToast.color, 2, gfx.color565(20, 30, 50));
    }
};

#endif // UI_MANAGER_H
