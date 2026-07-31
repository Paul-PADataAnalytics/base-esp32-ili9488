#ifndef UI_MANAGER_CORE_H
#define UI_MANAGER_CORE_H

#include <vector>
#include "UITheme.h"
#include "UIWidget.h"
#include "UILabel.h"
#include "UIButton.h"
#include "UICheckBox.h"
#include "UISlider.h"
#include "UIOptionSelector.h"
#include "UIFrame.h"
#include "../GFXContext.h"
#include "../Layer.h"

/**
 * Toast Notification Banner struct
 */
struct ToastMessage {
    String message;
    uint16_t color;
    float duration;
    float timer;
};

/**
 * UIManager — Root Manager for Touch UI Component Hierarchy & Toast Notifications.
 */
class UIManager {
private:
    std::vector<UIWidget*> _widgets;
    UITheme                _theme;
    ToastMessage           _activeToast;
    bool                   _hasToast;
    UIWidget*              _focusedWidget = nullptr;
    bool                   _wasTouched    = false;

public:
    UIManager() : _theme(UITheme_Dark()), _hasToast(false) {}

    void setTheme(const UITheme& theme) {
        _theme = theme;
        markAllDirty();
    }

    const UITheme& getTheme() const { return _theme; }

    void add(UIWidget* widget) {
        if (widget) {
            _widgets.push_back(widget);
            widget->clearDirty();
            markAllDirty();
        }
    }

    void clear() {
        _widgets.clear();
        _focusedWidget = nullptr;
    }

    String dumpUIJson() const {
        String json = "{\n  \"widgets\": [\n";
        for (size_t i = 0; i < _widgets.size(); i++) {
            if (_widgets[i]) {
                json += "    " + _widgets[i]->toJson();
                if (i < _widgets.size() - 1) json += ",";
                json += "\n";
            }
        }
        json += "  ]\n}";
        return json;
    }

    void markAllDirty() {
        for (auto* w : _widgets) {
            if (w) w->onTouchRelease(-1, -1);
        }
    }

    bool isDirty() const {
        for (const auto* w : _widgets) {
            if (w && w->isDirty()) return true;
        }
        return _hasToast;
    }

    void update(float deltaTime, GFXContext& gfx) {
        // 1. Toast timer
        if (_hasToast) {
            _activeToast.timer -= deltaTime;
            if (_activeToast.timer <= 0.0f) {
                _hasToast = false;
            }
        }

        // 2. Touch event handling & dispatch
        int tx = 0, ty = 0;
        bool currentlyTouched = gfx.getTouch(&tx, &ty);

        if (currentlyTouched && !_wasTouched) {
            // Touch Press
            _focusedWidget = nullptr;
            for (auto it = _widgets.rbegin(); it != _widgets.rend(); ++it) {
                UIWidget* w = *it;
                if (w && w->enabled && w->visible && w->hitTest(tx, ty)) {
                    if (w->onTouchPress(tx, ty)) {
                        _focusedWidget = w;
                        break;
                    }
                }
            }
        } else if (currentlyTouched && _wasTouched) {
            // Touch Drag
            if (_focusedWidget) {
                _focusedWidget->onTouchDrag(tx, ty);
            }
        } else if (!currentlyTouched && _wasTouched) {
            // Touch Release
            if (_focusedWidget) {
                _focusedWidget->onTouchRelease(tx, ty);
                _focusedWidget = nullptr;
            }
        }
        _wasTouched = currentlyTouched;

        // 3. Widget logic updates
        for (auto* w : _widgets) {
            if (w && w->visible) {
                w->update(deltaTime);
            }
        }
    }

    void draw(LGFX_Sprite* buf, int bandY, Layer& layer) {
        int bandH = buf ? buf->height() : 80;

        for (auto* w : _widgets) {
            if (w && w->visible && w->overlapsBand(bandY, bandH)) {
                w->draw(buf, bandY, bandH, _theme);
                w->clearDirty();
            }
        }

        renderToast(buf, bandY);
    }

    // --- Toast Notifications ---

    void showToast(const char* msg, uint16_t color = 0x07FF, float duration = 2.5f) {
        _activeToast.message  = msg;
        _activeToast.color    = color;
        _activeToast.duration = duration;
        _activeToast.timer    = duration;
        _hasToast = true;
    }

    void renderToast(LGFX_Sprite* buf, int bandY) {
        if (!_hasToast || !buf) return;

        int toastW = _activeToast.message.length() * 12 + 24;
        int toastH = 34;
        int toastX = (480 - toastW) / 2;
        int toastY = 40;

        if (toastY + toastH < bandY || toastY >= bandY + buf->height()) return;

        int ly = toastY - bandY;
        buf->fillRoundRect(toastX, ly, toastW, toastH, 8, _theme.background);
        buf->drawRoundRect(toastX, ly, toastW, toastH, 8, _activeToast.color);

        if (_theme.fontBody) buf->setFont(_theme.fontBody);
        buf->setTextColor(_activeToast.color);
        buf->setCursor(toastX + 12, ly + 8);
        buf->print(_activeToast.message.c_str());
    }
};

#endif // UI_MANAGER_CORE_H
