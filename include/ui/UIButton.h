#ifndef UI_BUTTON_H
#define UI_BUTTON_H

#include <functional>
#include "UIWidget.h"

/**
 * UIButton — Pressable button with text label.
 *
 * ## Visual States
 *   NORMAL   — surface colour, standard border
 *   PRESSED  — surfacePressed colour, focusedBorder, slight inset look
 *   DISABLED — surfaceDisabled, muted text, no touch interaction
 *
 * ## Callbacks
 *   button.onPressed([]() { doSomething(); });
 *   button.onLongPress([]() { showContextMenu(); });  // hold > 600 ms
 *
 * ## Optional Danger Styling
 *   button.setDanger(true);  // tints border red for destructive actions
 *
 * Usage:
 *   UIButton play(20, 140, 200, 44, "Play Game");
 *   play.onPressed([&]() { app.startGame(); });
 */
class UIButton : public UIWidget {
public:
    using Callback = std::function<void()>;

private:
    String   _label;
    Callback _onPressed;
    Callback _onLongPress;
    bool     _danger          = false;
    bool     _longPressFired  = false;
    const lgfx::IFont* _fontOverride = nullptr;

public:
    UIButton() = default;

    UIButton(int x, int y, int w, int h, const char* label = "Button")
        : UIWidget(x, y, w, h), _label(label) {}

    // --- Configuration ---
    void setLabel(const char* label)     { _label = label; markDirty(); }
    void setDanger(bool danger)          { _danger = danger; markDirty(); }
    void setFont(const lgfx::IFont* f)   { _fontOverride = f; markDirty(); }
    void onPressed(Callback cb)          { _onPressed   = cb; }
    void onLongPress(Callback cb)        { _onLongPress = cb; }

    const char* getLabel() const { return _label.c_str(); }

    String toJson() const override {
        char buf[256];
        snprintf(buf, sizeof(buf), "{\"type\":\"button\",\"x\":%d,\"y\":%d,\"w\":%d,\"h\":%d,\"label\":\"%s\",\"pressed\":%s,\"enabled\":%s}",
                 x, y, w, h, _label.c_str(), _pressed ? "true" : "false", enabled ? "true" : "false");
        return String(buf);
    }

    // --- Update (long-press timer) ---
    void update(float deltaTime) override {
        if (_pressed && _onLongPress && !_longPressFired) {
            if (millis() - _pressTime >= LONG_PRESS_MS) {
                _longPressFired = true;
                _pressed = false;
                markDirty();
                _onLongPress();
            }
        }
    }

    // --- Draw ---
    void draw(LGFX_Sprite* buf, int bandY, int bandH, const UITheme& theme) override {
        if (!visible || !buf || !overlapsBand(bandY, bandH)) return;

        int ly = y - bandY;
        uint16_t bg, border;

        if (!enabled) {
            bg     = theme.surfaceDisabled;
            border = theme.border;
        } else if (_pressed) {
            bg     = theme.surfacePressed;
            border = theme.borderFocused;
        } else {
            bg     = theme.surface;
            border = _danger ? theme.danger : theme.border;
        }

        // Fill
        buf->fillRoundRect(x, ly, w, h, theme.cornerRadius, bg);
        // Border (draw twice for thickness > 1)
        for (int i = 0; i < theme.borderWidth; i++) {
            buf->drawRoundRect(x + i, ly + i, w - i*2, h - i*2, theme.cornerRadius, border);
        }
        // Pressed inset shadow line (top/left darker edge)
        if (_pressed && enabled) {
            buf->drawFastHLine(x + theme.cornerRadius, ly + 1, w - theme.cornerRadius*2, theme.accentDark);
        }

        // Label text (centered)
        const lgfx::IFont* font = _fontOverride ? _fontOverride : theme.fontBody;
        uint16_t textCol = enabled ? theme.textPrimary : theme.textSecondary;
        if (_danger && enabled && !_pressed) textCol = theme.danger;

        drawCenteredText(buf, bandY, x, y, w, h, _label.c_str(), textCol, font);
    }

    // --- Touch ---
    bool onTouchPress(int16_t tx, int16_t ty) override {
        if (!enabled || !visible || !hitTest(tx, ty)) return false;
        _pressed         = true;
        _longPressFired  = false;
        _pressTime       = millis();
        _pressX          = tx;
        _pressY          = ty;
        markDirty();
        return true;
    }

    bool onTouchRelease(int16_t tx, int16_t ty) override {
        if (!_pressed) return false;
        bool wasInside = hitTest(tx, ty);
        _pressed = false;
        markDirty();
        if (wasInside && !_longPressFired && _onPressed) {
            _onPressed();
        }
        _longPressFired = false;
        return true;
    }

    bool onTouchDrag(int16_t tx, int16_t ty) override {
        // Cancel press if drag leaves button bounds
        if (_pressed && !hitTest(tx, ty)) {
            _pressed = false;
            markDirty();
        }
        return false;  // Don't consume drag — let scroll handle it
    }
};

#endif // UI_BUTTON_H
