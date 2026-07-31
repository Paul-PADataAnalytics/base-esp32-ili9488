#ifndef UI_FRAME_H
#define UI_FRAME_H

#include <vector>
#include "UIWidget.h"

/**
 * UIFrame — Container widget for grouping child widgets with title bar and scroll support.
 */
class UIFrame : public UIWidget {
private:
    String                 _title;
    std::vector<UIWidget*> _children;
    UIWidget*              _activeTouchChild = nullptr;
    int                    _scrollOffsetY    = 0;
    int                    _maxScrollY       = 0;
    bool                   _showBorder       = true;
    bool                   _isDragging       = false;
    int16_t                _lastTouchY       = 0;

public:
    UIFrame() = default;

    UIFrame(int x, int y, int w, int h, const char* title = "")
        : UIWidget(x, y, w, h), _title(title) {}

    void setTitle(const char* title) {
        _title = title;
        markDirty();
    }

    void add(UIWidget* child) {
        if (child) {
            _children.push_back(child);
            updateContentLayout();
            markDirty();
        }
    }

    void clear() {
        _children.clear();
        _activeTouchChild = nullptr;
        _scrollOffsetY = 0;
        _maxScrollY = 0;
        markDirty();
    }

    String toJson() const override {
        char buf[256];
        snprintf(buf, sizeof(buf), "{\"type\":\"frame\",\"x\":%d,\"y\":%d,\"w\":%d,\"h\":%d,\"title\":\"%s\",\"children\":[",
                 x, y, w, h, _title.c_str());
        String json = buf;
        for (size_t i = 0; i < _children.size(); i++) {
            if (_children[i]) {
                json += _children[i]->toJson();
                if (i < _children.size() - 1) json += ",";
            }
        }
        json += "]}";
        return json;
    }

    void update(float deltaTime) override {
        for (auto* child : _children) {
            if (child->visible) {
                child->update(deltaTime);
                if (child->isDirty()) {
                    markDirty();
                }
            }
        }
    }

    void draw(LGFX_Sprite* buf, int bandY, int bandH, const UITheme& theme) override {
        if (!visible || !buf || !overlapsBand(bandY, bandH)) return;

        int ly = y - bandY;
        int titleH = _title.length() > 0 ? 28 : 0;

        // Container Background
        buf->fillRoundRect(x, ly, w, h, theme.cornerRadius, theme.background);
        if (_showBorder) {
            buf->drawRoundRect(x, ly, w, h, theme.cornerRadius, theme.border);
        }

        // Title Bar
        if (titleH > 0) {
            buf->fillRoundRect(x, ly, w, titleH, theme.cornerRadius, theme.titleBar);
            buf->fillRect(x, ly + titleH - theme.cornerRadius, w, theme.cornerRadius, theme.titleBar);
            if (_showBorder) {
                buf->drawFastHLine(x, ly + titleH, w, theme.border);
            }

            const lgfx::IFont* font = theme.fontHeading ? theme.fontHeading : theme.fontBody;
            drawLeftText(buf, bandY, x, y, titleH, theme.paddingX, _title.c_str(), theme.titleText, font);
        }

        // Render Child Widgets
        for (auto* child : _children) {
            if (child->visible) {
                int originalY = child->y;
                child->y -= _scrollOffsetY;

                if (child->y + child->h > y + titleH && child->y < y + h) {
                    child->draw(buf, bandY, bandH, theme);
                }

                child->y = originalY;
            }
        }
    }

    bool onTouchPress(int16_t tx, int16_t ty) override {
        if (!enabled || !visible || !hitTest(tx, ty)) return false;

        _isDragging = true;
        _lastTouchY = ty;
        _activeTouchChild = nullptr;

        // Forward to child widgets (considering scroll offset)
        int titleH = _title.length() > 0 ? 28 : 0;
        if (ty >= y + titleH) {
            for (auto* child : _children) {
                if (child->enabled && child->visible) {
                    int originalY = child->y;
                    child->y -= _scrollOffsetY;

                    if (child->hitTest(tx, ty)) {
                        bool consumed = child->onTouchPress(tx, ty);
                        child->y = originalY;
                        if (consumed) {
                            _activeTouchChild = child;
                            return true;
                        }
                    }
                    child->y = originalY;
                }
            }
        }
        return true;
    }

    bool onTouchDrag(int16_t tx, int16_t ty) override {
        if (!_isDragging) return false;

        // Forward drag to active child (e.g. UISlider) if present
        if (_activeTouchChild) {
            int originalY = _activeTouchChild->y;
            _activeTouchChild->y -= _scrollOffsetY;
            bool consumed = _activeTouchChild->onTouchDrag(tx, ty);
            _activeTouchChild->y = originalY;
            if (consumed) return true;
        }

        // Otherwise perform container vertical scroll
        int dy = ty - _lastTouchY;
        _lastTouchY = ty;

        if (_maxScrollY > 0 && dy != 0) {
            _scrollOffsetY -= dy;
            if (_scrollOffsetY < 0) _scrollOffsetY = 0;
            if (_scrollOffsetY > _maxScrollY) _scrollOffsetY = _maxScrollY;
            markDirty();
            return true;
        }

        return false;
    }

    bool onTouchRelease(int16_t tx, int16_t ty) override {
        _isDragging = false;
        if (_activeTouchChild) {
            int originalY = _activeTouchChild->y;
            _activeTouchChild->y -= _scrollOffsetY;
            _activeTouchChild->onTouchRelease(tx, ty);
            _activeTouchChild->y = originalY;
            _activeTouchChild = nullptr;
        } else {
            for (auto* child : _children) {
                if (child->enabled && child->visible) {
                    int originalY = child->y;
                    child->y -= _scrollOffsetY;
                    child->onTouchRelease(tx, ty);
                    child->y = originalY;
                }
            }
        }
        return true;
    }

private:
    void updateContentLayout() {
        int titleH = _title.length() > 0 ? 28 : 0;
        int totalContentH = titleH;

        for (auto* child : _children) {
            int childBottom = child->y + child->h - y;
            if (childBottom > totalContentH) {
                totalContentH = childBottom;
            }
        }

        _maxScrollY = totalContentH - h;
        if (_maxScrollY < 0) _maxScrollY = 0;
    }
};

#endif // UI_FRAME_H
