#ifndef GFX_CONTEXT_H
#define GFX_CONTEXT_H

#include <Arduino.h>
#include <LovyanGFX.hpp>

/**
 * GFXContext - Hardware Abstraction Layer Drawing & Input API
 * 
 * Provides high-level drawing, color math, sprite double-buffering,
 * touch screen input, and FPS metrics while hiding low-level hardware.
 */
class GFXContext {
private:
    lgfx::LGFX_Device* _lcd;
    LGFX_Sprite*       _sprite;
    float              _fps;

    int _spriteX;
    int _spriteY;
    int _spriteW;
    int _spriteH;

public:
    GFXContext(lgfx::LGFX_Device* lcd, LGFX_Sprite* sprite, int sx, int sy, int sw, int sh)
        : _lcd(lcd), _sprite(sprite), _fps(0.0f), _spriteX(sx), _spriteY(sy), _spriteW(sw), _spriteH(sh) {}

    void setFPS(float fps) { _fps = fps; }
    float getFPS() const { return _fps; }

    int getWidth() const { return 480; }
    int getHeight() const { return 320; }
    int getSpriteX() const { return _spriteX; }
    int getSpriteY() const { return _spriteY; }
    int getSpriteW() const { return _spriteW; }
    int getSpriteH() const { return _spriteH; }

    // --- Touch Screen API ---
    // Returns true if touched, populating x and y with touch coordinates
    bool getTouch(int* x, int* y) {
        return _lcd->getTouch(x, y);
    }

    bool isTouched() {
        int x, y;
        return _lcd->getTouch(&x, &y);
    }

    // --- Drawing API ---
    uint16_t color565(uint8_t r, uint8_t g, uint8_t b) {
        return _lcd->color565(r, g, b);
    }

    bool overlapsBuffer(float x, float y, float r) const {
        return (x + r + 4 >= _spriteX &&
                x - r - 4 <= _spriteX + _spriteW &&
                y + r + 4 >= _spriteY &&
                y - r - 4 <= _spriteY + _spriteH);
    }

    void fillScreen(uint16_t color) {
        _lcd->fillScreen(color);
    }

    void drawCircleDirect(int x, int y, int r, uint16_t color, bool fill = true) {
        if (fill) _lcd->fillCircle(x, y, r, color);
        else _lcd->drawCircle(x, y, r, color);
    }

    void eraseCircleDirect(int x, int y, int r, uint16_t color) {
        _lcd->fillCircle(x, y, r + 2, color);
    }

    void drawRectDirect(int x, int y, int w, int h, uint16_t color) {
        _lcd->drawRect(x, y, w, h, color);
    }

    void drawTextDirect(const char* text, int x, int y, uint16_t color, uint8_t size = 2) {
        _lcd->setTextColor(color, 0x0813);
        _lcd->setTextSize(size);
        _lcd->setCursor(x, y);
        _lcd->print(text);
    }

    void clearBuffer(uint16_t color = 0x0813) {
        _sprite->fillScreen(color);
    }

    void drawLineBuffer(int x1, int y1, int x2, int y2, uint16_t color) {
        _sprite->drawLine(x1, y1, x2, y2, color);
    }

    void drawCircleBuffer(int x, int y, int r, uint16_t color, bool fill = true) {
        if (fill) _sprite->fillCircle(x, y, r, color);
        else _sprite->drawCircle(x, y, r, color);
    }

    void pushBuffer() {
        _sprite->pushSprite(_spriteX, _spriteY);
    }
};

#endif // GFX_CONTEXT_H
