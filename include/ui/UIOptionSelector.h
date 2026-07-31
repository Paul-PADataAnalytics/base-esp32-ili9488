#ifndef UI_OPTION_SELECTOR_H
#define UI_OPTION_SELECTOR_H

#include <vector>
#include <functional>
#include "UIWidget.h"

/**
 * UIOptionSelector — Radio button style mutually exclusive option selector widget.
 */
class UIOptionSelector : public UIWidget {
public:
    using ChangeCallback = std::function<void(int)>;

private:
    std::vector<String> _options;
    int                 _selectedIndex = 0;
    int*                _binding       = nullptr;
    ChangeCallback      _onChange;
    String              _label;

public:
    UIOptionSelector() = default;

    UIOptionSelector(int x, int y, int w, int h, const char* label = "")
        : UIWidget(x, y, w, h), _label(label) {}

    void addOption(const char* opt) {
        _options.push_back(String(opt));
        markDirty();
    }

    void clearOptions() {
        _options.clear();
        _selectedIndex = 0;
        markDirty();
    }

    void setSelected(int index) {
        if (index >= 0 && index < (int)_options.size() && _selectedIndex != index) {
            _selectedIndex = index;
            if (_binding) *_binding = _selectedIndex;
            if (_onChange) _onChange(_selectedIndex);
            markDirty();
        }
    }

    int getSelectedIndex() const { return _selectedIndex; }
    const char* getSelectedOption() const {
        if (_selectedIndex >= 0 && _selectedIndex < (int)_options.size()) {
            return _options[_selectedIndex].c_str();
        }
        return "";
    }

    String toJson() const override {
        char buf[256];
        snprintf(buf, sizeof(buf), "{\"type\":\"option_selector\",\"x\":%d,\"y\":%d,\"w\":%d,\"h\":%d,\"label\":\"%s\",\"selected\":%d,\"selectedOption\":\"%s\",\"enabled\":%s}",
                 x, y, w, h, _label.c_str(), _selectedIndex, getSelectedOption(), enabled ? "true" : "false");
        return String(buf);
    }

    void onChanged(ChangeCallback cb) { _onChange = cb; }

    void bindTo(int* valPtr) {
        _binding = valPtr;
        if (valPtr) {
            _selectedIndex = *valPtr;
            markDirty();
        }
    }

    void draw(LGFX_Sprite* buf, int bandY, int bandH, const UITheme& theme) override {
        if (!visible || !buf || !overlapsBand(bandY, bandH) || _options.empty()) return;

        int ly = y - bandY;
        int labelHeight = _label.length() > 0 ? 18 : 0;

        if (_label.length() > 0) {
            uint16_t textCol = enabled ? theme.textPrimary : theme.textSecondary;
            drawLeftText(buf, bandY, x, y, labelHeight, 0, _label.c_str(), textCol, theme.fontBody);
        }

        int optionsY = ly + labelHeight;
        int optionsH = h - labelHeight;
        int count = (int)_options.size();
        int optW = (w - (count - 1) * theme.spacing) / count;

        for (int i = 0; i < count; i++) {
            int optX = x + i * (optW + theme.spacing);
            bool isSel = (i == _selectedIndex);

            uint16_t bg = isSel ? (enabled ? theme.accent : theme.surfaceDisabled) : theme.surface;
            uint16_t border = isSel ? theme.borderFocused : theme.border;
            uint16_t textCol = isSel ? (enabled ? theme.background : theme.textSecondary) : (enabled ? theme.textPrimary : theme.textSecondary);

            buf->fillRoundRect(optX, optionsY, optW, optionsH, theme.cornerRadius, bg);
            buf->drawRoundRect(optX, optionsY, optW, optionsH, theme.cornerRadius, border);

            drawCenteredText(buf, bandY, optX, y + labelHeight, optW, optionsH, _options[i].c_str(), textCol, theme.fontBody);
        }
    }

    bool onTouchPress(int16_t tx, int16_t ty) override {
        if (!enabled || !visible || !hitTest(tx, ty) || _options.empty()) return false;

        int labelHeight = _label.length() > 0 ? 18 : 0;
        if (ty < y + labelHeight) return false;

        int count = (int)_options.size();
        int optW = (w - (count - 1) * 8) / count;

        for (int i = 0; i < count; i++) {
            int optX = x + i * (optW + 8);
            if (tx >= optX && tx < optX + optW) {
                setSelected(i);
                return true;
            }
        }
        return false;
    }
};

#endif // UI_OPTION_SELECTOR_H
