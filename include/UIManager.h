#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <Arduino.h>
#include <LovyanGFX.hpp>
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
 * 
 * All UI widgets render directly into the off-screen sprite buffer for 0% flicker.
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

    // --- UI Component Primitives Rendering into Sprite Buffer ---

    void drawProgressBar(LGFX_Sprite* buffer, int x, int y, int w, int h, float progress, uint16_t barColor, uint16_t bgColor = 0x18C3) {
        if (!buffer) return;
        progress = constrain(progress, 0.0f, 1.0f);
        buffer->fillRoundRect(x, y, w, h, h / 2, bgColor);
        buffer->drawRoundRect(x, y, w, h, h / 2, 0xFFFF);

        int fillW = (int)((w - 4) * progress);
        if (fillW > 0) {
            buffer->fillRoundRect(x + 2, y + 2, fillW, h - 4, (h - 4) / 2, barColor);
        }
    }

    // Render Active Toast Notification into Sprite Buffer
    void renderToast(LGFX_Sprite* buffer) {
        if (!_hasToast || !buffer) return;

        int toastW = _activeToast.message.length() * 12 + 24;
        int toastH = 34;
        int toastX = (buffer->width() - toastW) / 2;
        int toastY = 40;

        buffer->fillRoundRect(toastX, toastY, toastW, toastH, 8, 0x11C7);
        buffer->drawRoundRect(toastX, toastY, toastW, toastH, 8, _activeToast.color);
        buffer->setTextColor(_activeToast.color, 0x11C7);
        buffer->setTextSize(2);
        buffer->setCursor(toastX + 12, toastY + 8);
        buffer->print(_activeToast.message.c_str());
    }
};

#endif // UI_MANAGER_H
