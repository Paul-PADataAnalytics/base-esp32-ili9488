#ifndef UI_LABEL_H
#define UI_LABEL_H

#include "UIWidget.h"

/**
 * UILabel — Static or dynamic text display widget.
 *
 * Supports left, centre, and right alignment. Automatically marks dirty
 * whenever text or colour is changed via the setters.
 *
 * Usage:
 *   UILabel title(10, 10, 460, 30, "Settings");
 *   title.setAlign(UILabel::ALIGN_CENTER);
 *   title.setFont(&fonts::FreeSansBold12pt7b);
 *
 *   // Dynamic update (marks dirty automatically):
 *   label.setText(myDynamicString.c_str());
 */
class UILabel : public UIWidget {
public:
    static constexpr int ALIGN_LEFT   = 0;
    static constexpr int ALIGN_CENTER = 1;
    static constexpr int ALIGN_RIGHT  = 2;

private:
    String              _text;
    int                 _align         = ALIGN_LEFT;
    uint16_t            _color         = 0xFFFF;
    const lgfx::IFont*  _fontOverride  = nullptr;  // nullptr = use theme.fontBody
    bool                _useThemeColor = true;      // if false, use _color

public:
    UILabel() = default;

    UILabel(int x, int y, int w, int h, const char* text = "")
        : UIWidget(x, y, w, h), _text(text) {}

    // --- Setters (all auto-dirty) ---

    void setText(const char* text) {
        if (_text != text) { _text = text; markDirty(); }
    }

    void setAlign(int align) {
        if (_align != align) { _align = align; markDirty(); }
    }

    /** Override font for this label. Pass nullptr to fall back to theme.fontBody. */
    void setFont(const lgfx::IFont* font) {
        if (_fontOverride != font) { _fontOverride = font; markDirty(); }
    }

    /** Set a fixed colour, bypassing the theme's textPrimary. */
    void setColor(uint16_t color) {
        _color = color; _useThemeColor = false; markDirty();
    }

    /** Restore to using the theme's textPrimary colour. */
    void useThemeColor() { _useThemeColor = true; markDirty(); }

    const char* getText() const { return _text.c_str(); }

    String toJson() const override {
        char buf[256];
        snprintf(buf, sizeof(buf), "{\"type\":\"label\",\"x\":%d,\"y\":%d,\"w\":%d,\"h\":%d,\"text\":\"%s\"}",
                 x, y, w, h, _text.c_str());
        return String(buf);
    }

    // --- Draw ---

    void draw(LGFX_Sprite* buf, int bandY, int bandH, const UITheme& theme) override {
        if (!visible || !buf || !overlapsBand(bandY, bandH)) return;

        const lgfx::IFont* font = _fontOverride ? _fontOverride : theme.fontBody;
        uint16_t           col  = _useThemeColor ? (enabled ? theme.textPrimary : theme.textSecondary) : _color;

        if (font) buf->setFont(font);
        buf->setTextColor(col);

        int th = buf->fontHeight();
        int ly = y - bandY + (h - th) / 2;

        switch (_align) {
            case ALIGN_CENTER: {
                int tw = buf->textWidth(_text.c_str());
                buf->setCursor(x + (w - tw) / 2, ly);
                break;
            }
            case ALIGN_RIGHT: {
                int tw = buf->textWidth(_text.c_str());
                buf->setCursor(x + w - tw, ly);
                break;
            }
            default: // ALIGN_LEFT
                buf->setCursor(x, ly);
                break;
        }
        buf->print(_text.c_str());
    }
};

#endif // UI_LABEL_H
