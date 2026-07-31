#ifndef UI_WIDGET_H
#define UI_WIDGET_H

#if defined(PLATFORM_LINUX) || !defined(ARDUINO)
#include "Arduino_Linux.h"
#else
#include <Arduino.h>
#endif
#include <LovyanGFX.hpp>
#include <functional>
#include "UITheme.h"

/**
 * UIWidget — Abstract base class for all UI widgets.
 *
 * ## Coordinate System
 * All widget coordinates (x, y, w, h) are in canvas-absolute pixels.
 * The engine translates to band-local coordinates before calling draw().
 *
 * ## Dirty Tracking
 * Each widget tracks its own dirty state. When visual content changes,
 * call markDirty(). UIManager aggregates all widget dirty flags and
 * notifies the owning Layer to trigger a re-render.
 *
 * ## Touch State Machine
 *
 *   IDLE ──[onTouchPress inside]──► PRESSED
 *   PRESSED ──[onTouchRelease inside]──► fires callback, → IDLE
 *   PRESSED ──[onTouchDrag]──► widget-specific (scroll, slider drag)
 *   PRESSED ──[onTouchRelease outside]──► cancelled, → IDLE
 *   PRESSED ──[hold > LONG_PRESS_MS]──► fires long-press, → IDLE
 *
 * ## Subclassing
 * Override draw(), and whichever touch methods your widget needs.
 * Call markDirty() whenever visual state changes inside your widget.
 */
class UIWidget {
public:
    // Canvas-absolute bounds
    int16_t x = 0, y = 0, w = 0, h = 0;
    bool    visible = true;
    bool    enabled = true;

protected:
    bool          _dirty      = true;   // Needs redraw this frame
    bool          _pressed    = false;  // Currently held down
    unsigned long _pressTime  = 0;      // millis() at press start
    int16_t       _pressX     = 0;
    int16_t       _pressY     = 0;

    static constexpr unsigned long LONG_PRESS_MS = 600;

public:
    UIWidget() = default;
    UIWidget(int x, int y, int w, int h) : x(x), y(y), w(w), h(h) {}
    virtual ~UIWidget() = default;

    // -------------------------------------------------------------------------
    // Subclass interface
    // -------------------------------------------------------------------------

    /** Per-frame logic update (animations, long-press timer, etc.) */
    virtual void update(float deltaTime) {}

    /**
     * Draw widget content into the band sprite buffer.
     *
     * @param buf    The active band sprite (480 × bandHeight px).
     * @param bandY  Top canvas-Y of this band (subtract from canvas coords for local coords).
     * @param bandH  Height of the band sprite in pixels.
     * @param theme  Active UI theme.
     */
    virtual void draw(LGFX_Sprite* buf, int bandY, int bandH, const UITheme& theme) = 0;

    /**
     * Touch press event. Return true if widget consumes it (prevents bubbling).
     * tx, ty are canvas-absolute coordinates.
     */
    virtual bool onTouchPress(int16_t tx, int16_t ty) { return false; }

    /** Touch release event (canvas-absolute tx, ty). */
    virtual bool onTouchRelease(int16_t tx, int16_t ty) { return false; }

    /** Touch drag event (canvas-absolute tx, ty). */
    virtual bool onTouchDrag(int16_t tx, int16_t ty) { return false; }

    // -------------------------------------------------------------------------
    // Dirty State
    // -------------------------------------------------------------------------
    bool isDirty()    const { return _dirty && visible; }
    void clearDirty()       { _dirty = false; }

    // -------------------------------------------------------------------------
    // Geometry Helpers
    // -------------------------------------------------------------------------

    /** Returns true if canvas-absolute (tx, ty) falls inside this widget's bounds. */
    bool hitTest(int16_t tx, int16_t ty) const {
        return tx >= x && tx < x + w && ty >= y && ty < y + h;
    }

    /** Returns true if this widget overlaps the band [bandY, bandY+bandH). */
    bool overlapsBand(int bandY, int bandH) const {
        return (y < bandY + bandH) && (y + h > bandY);
    }

    /**
     * Translate a canvas Y coordinate to band-local Y.
     * Band-local Y is what LGFX_Sprite drawing methods expect.
     */
    int toLocalY(int canvasY, int bandY) const { return canvasY - bandY; }

    // -------------------------------------------------------------------------
    // Protected Helpers (for subclasses)
    // -------------------------------------------------------------------------
protected:
    void markDirty() { _dirty = true; }

    /**
     * Draw a centered text string inside a rect, clipped to the band.
     * Returns the measured text width.
     */
    int drawCenteredText(LGFX_Sprite* buf, int bandY,
                         int rx, int ry, int rw, int rh,
                         const char* text, uint16_t color,
                         const lgfx::IFont* font) const {
        if (!buf || !text) return 0;
        if (font) buf->setFont(font);
        buf->setTextColor(color);
        int tw = buf->textWidth(text);
        int th = buf->fontHeight();
        int lx = rx + (rw - tw) / 2;
        int ly = ry - bandY + (rh - th) / 2;
        buf->setCursor(lx, ly);
        buf->print(text);
        return tw;
    }

    /**
     * Draw left-aligned text inside a rect, with optional X offset.
     */
    void drawLeftText(LGFX_Sprite* buf, int bandY,
                      int rx, int ry, int rh, int xOffset,
                      const char* text, uint16_t color,
                      const lgfx::IFont* font) const {
        if (!buf || !text) return;
        if (font) buf->setFont(font);
        buf->setTextColor(color);
        int th = buf->fontHeight();
        buf->setCursor(rx + xOffset, ry - bandY + (rh - th) / 2);
        buf->print(text);
    }

    /**
     * Draw right-aligned text inside a rect.
     */
    void drawRightText(LGFX_Sprite* buf, int bandY,
                       int rx, int ry, int rw, int rh, int xOffset,
                       const char* text, uint16_t color,
                       const lgfx::IFont* font) const {
        if (!buf || !text) return;
        if (font) buf->setFont(font);
        buf->setTextColor(color);
        int tw = buf->textWidth(text);
        int th = buf->fontHeight();
        buf->setCursor(rx + rw - tw - xOffset, ry - bandY + (rh - th) / 2);
        buf->print(text);
    }
};

#endif // UI_WIDGET_H
