#ifndef UI_CHECKBOX_H
#define UI_CHECKBOX_H

#include <functional>
#include "UIWidget.h"

/**
 * UICheckBox — Boolean toggle widget with label.
 *
 * Renders a rounded tick-box on the left and a label on the right.
 * Tapping anywhere within the widget's bounds toggles the state.
 *
 * ## Data Binding
 *
 * Callback style (fire-and-forget):
 *   checkbox.onChanged([](bool v) { settings.sfx = v; });
 *
 * Pointer binding (two-way, reads initial value + writes back):
 *   checkbox.bindTo(&settings.sfx);
 *
 * Both can be used together; the callback fires whenever the value changes,
 * whether triggered by touch or by code.
 *
 * Usage:
 *   UICheckBox sfxToggle(20, 80, 300, 40, "Sound Effects", true);
 *   sfxToggle.bindTo(&game.settings.sfx);
 *   sfxToggle.onChanged([](bool v) { audio.setSfxEnabled(v); });
 */
class UICheckBox : public UIWidget {
public:
    using ChangeCallback = std::function<void(bool)>;

private:
    String         _label;
    bool           _checked  = false;
    bool*          _binding  = nullptr;
    ChangeCallback _onChange;

public:
    UICheckBox() = default;

    UICheckBox(int x, int y, int w, int h, const char* label, bool initialValue = false)
        : UIWidget(x, y, w, h), _label(label), _checked(initialValue) {}

    // --- Configuration ---
    void setLabel(const char* label) { _label = label; markDirty(); }

    void onChanged(ChangeCallback cb) { _onChange = cb; }

    /**
     * Bind to a bool variable pointer (two-way).
     * Reads the initial value from *value immediately.
     * Writes back to *value whenever toggled.
     */
    void bindTo(bool* value) {
        _binding = value;
        if (value) { _checked = *value; markDirty(); }
    }

    /** Set state programmatically (fires callback + updates binding). */
    void setChecked(bool v) {
        if (_checked == v) return;
        _checked = v;
        if (_binding) *_binding = v;
        if (_onChange) _onChange(v);
        markDirty();
    }

    bool isChecked() const { return _checked; }

    String toJson() const override {
        char buf[256];
        snprintf(buf, sizeof(buf), "{\"type\":\"checkbox\",\"x\":%d,\"y\":%d,\"w\":%d,\"h\":%d,\"label\":\"%s\",\"checked\":%s,\"enabled\":%s}",
                 x, y, w, h, _label.c_str(), _checked ? "true" : "false", enabled ? "true" : "false");
        return String(buf);
    }

    // --- Draw ---
    void draw(LGFX_Sprite* buf, int bandY, int bandH, const UITheme& theme) override {
        if (!visible || !buf || !overlapsBand(bandY, bandH)) return;

        int boxSz = theme.checkSize;
        int ly    = y - bandY;
        int boxY  = ly + (h - boxSz) / 2;

        // Box background
        uint16_t bgCol  = enabled ? theme.surface : theme.surfaceDisabled;
        uint16_t bdrCol = _checked && enabled ? theme.accent : theme.border;

        buf->fillRoundRect(x, boxY, boxSz, boxSz, theme.cornerRadius / 2, bgCol);
        buf->drawRoundRect(x, boxY, boxSz, boxSz, theme.cornerRadius / 2, bdrCol);

        // Tick mark (bold double-line ✓)
        if (_checked) {
            uint16_t tickCol = enabled ? theme.accent : theme.textSecondary;
            int tx1 = x + 3,         ty1 = boxY + boxSz / 2 + 1;
            int tx2 = x + boxSz / 2 - 1, ty2 = boxY + boxSz - 4;
            int tx3 = x + boxSz - 3, ty3 = boxY + 4;
            // Draw tick twice offset by 1px for bold effect
            for (int d = 0; d < 2; d++) {
                buf->drawLine(tx1, ty1 + d, tx2, ty2 + d, tickCol);
                buf->drawLine(tx2, ty2 + d, tx3, ty3 + d, tickCol);
            }
        }

        // Label
        uint16_t textCol = enabled ? theme.textPrimary : theme.textSecondary;
        drawLeftText(buf, bandY, x, y, h, boxSz + theme.paddingX, _label.c_str(), textCol, theme.fontBody);
    }

    // --- Touch ---
    bool onTouchPress(int16_t tx, int16_t ty) override {
        if (!enabled || !visible || !hitTest(tx, ty)) return false;
        setChecked(!_checked);
        return true;
    }
};

#endif // UI_CHECKBOX_H
