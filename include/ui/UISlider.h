#ifndef UI_SLIDER_H
#define UI_SLIDER_H

#include <cmath>
#include <functional>
#include "UIWidget.h"

/**
 * UISlider — Horizontal draggable slider widget.
 *
 * Supports range configuration, callbacks, pointer binding, and continuous touch drag.
 */
class UISlider : public UIWidget {
public:
    using ChangeCallback = std::function<void(float)>;

private:
    float          _minVal   = 0.0f;
    float          _maxVal   = 1.0f;
    float          _val      = 0.5f;
    float*         _binding  = nullptr;
    ChangeCallback _onChange;
    String         _label;
    bool           _showValue = true;

public:
    UISlider() = default;

    UISlider(int x, int y, int w, int h, const char* label = "", float minV = 0.0f, float maxV = 1.0f, float initialV = 0.5f)
        : UIWidget(x, y, w, h), _minVal(minV), _maxVal(maxV), _val(initialV), _label(label) {}

    void setRange(float minV, float maxV) {
        _minVal = minV;
        _maxVal = maxV;
        setValue(_val);
    }

    void setValue(float v) {
        if (v < _minVal) v = _minVal;
        if (v > _maxVal) v = _maxVal;
        // Threshold check (0.5% change minimum) to prevent touch jitter flooding Lua callbacks
        if (std::abs(_val - v) >= 0.005f * (_maxVal - _minVal) || v == _minVal || v == _maxVal) {
            _val = v;
            if (_binding) *_binding = _val;
            if (_onChange) _onChange(_val);
            markDirty();
        }
    }

    float getValue() const { return _val; }

    void onChanged(ChangeCallback cb) { _onChange = cb; }

    void bindTo(float* valPtr) {
        _binding = valPtr;
        if (valPtr) {
            _val = *valPtr;
            markDirty();
        }
    }

    void setShowValue(bool show) {
        if (_showValue != show) {
            _showValue = show;
            markDirty();
        }
    }

    void draw(LGFX_Sprite* buf, int bandY, int bandH, const UITheme& theme) override {
        if (!visible || !buf || !overlapsBand(bandY, bandH)) return;

        int ly = y - bandY;
        int trackH = theme.sliderTrackH;
        int thumbR = theme.sliderThumbR;

        int labelHeight = _label.length() > 0 ? 18 : 0;
        int trackY = ly + labelHeight + (h - labelHeight - trackH) / 2;

        // Draw Label and Value if provided
        if (_label.length() > 0) {
            uint16_t textCol = enabled ? theme.textPrimary : theme.textSecondary;
            drawLeftText(buf, bandY, x, y, labelHeight, 0, _label.c_str(), textCol, theme.fontBody);

            if (_showValue) {
                char valStr[16];
                snprintf(valStr, sizeof(valStr), "%.2f", _val);
                drawRightText(buf, bandY, x, y, w, labelHeight, 0, valStr, theme.textSecondary, theme.fontBody);
            }
        }

        int trackX = x + thumbR;
        int trackW = w - 2 * thumbR;
        if (trackW < 10) trackW = 10;

        // Track Background
        buf->fillRoundRect(trackX, trackY, trackW, trackH, trackH / 2, theme.surface);
        buf->drawRoundRect(trackX, trackY, trackW, trackH, trackH / 2, theme.border);

        // Filled Portion
        float pct = (_maxVal > _minVal) ? (_val - _minVal) / (_maxVal - _minVal) : 0.0f;
        int fillW = (int)(trackW * pct);
        if (fillW > 0) {
            uint16_t fillCol = enabled ? theme.accent : theme.surfaceDisabled;
            buf->fillRoundRect(trackX, trackY, fillW, trackH, trackH / 2, fillCol);
        }

        // Thumb
        int thumbX = trackX + fillW;
        int thumbY = trackY + trackH / 2;
        uint16_t thumbCol = enabled ? (_pressed ? theme.surfacePressed : theme.surface) : theme.surfaceDisabled;
        uint16_t thumbBorder = enabled ? (_pressed ? theme.borderFocused : theme.accent) : theme.border;

        buf->fillCircle(thumbX, thumbY, thumbR, thumbCol);
        buf->drawCircle(thumbX, thumbY, thumbR, thumbBorder);
    }

    bool onTouchPress(int16_t tx, int16_t ty) override {
        if (!enabled || !visible || !hitTest(tx, ty)) return false;
        _pressed = true;
        updateValueFromTouch(tx);
        return true;
    }

    bool onTouchDrag(int16_t tx, int16_t ty) override {
        if (!_pressed) return false;
        updateValueFromTouch(tx);
        return true;
    }

    bool onTouchRelease(int16_t tx, int16_t ty) override {
        if (!_pressed) return false;
        _pressed = false;
        markDirty();
        return true;
    }

private:
    void updateValueFromTouch(int16_t tx) {
        int thumbR = 9;
        int trackX = x + thumbR;
        int trackW = w - 2 * thumbR;
        if (trackW <= 0) return;

        float relX = (float)(tx - trackX) / (float)trackW;
        if (relX < 0.0f) relX = 0.0f;
        if (relX > 1.0f) relX = 1.0f;

        float newVal = _minVal + relX * (_maxVal - _minVal);
        setValue(newVal);
    }
};

#endif // UI_SLIDER_H
