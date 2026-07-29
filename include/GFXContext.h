#ifndef GFX_CONTEXT_H
#define GFX_CONTEXT_H

#include <Arduino.h>
#include <LovyanGFX.hpp>

/**
 * GFXContext - Hardware Abstraction Layer Drawing, Input, Image & Power API
 * 
 * Provides high-level drawing, 16-bit image/bitmap loading & rendering,
 * color math, sprite double-buffering, touch screen input, power/sleep control,
 * and UI primitives while hiding low-level hardware.
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

    // --- Display Power & Backlight Control ---
    void turnOnBacklight() {
        digitalWrite(32, HIGH);
    }

    void turnOffBacklight() {
        digitalWrite(32, LOW);
    }

    void sleepDisplay() {
        _lcd->sleep();
    }

    void wakeDisplay() {
        _lcd->wakeup();
    }

    // --- Touch Screen API ---
    bool getTouch(int* x, int* y) {
        return _lcd->getTouch(x, y);
    }

    bool isTouched() {
        int x, y;
        return _lcd->getTouch(&x, &y);
    }

    // --- Image & Bitmap Rendering Primitives ---

    /**
     * Renders a 16-bit RGB565 bitmap array directly to the display.
     */
    void pushImageDirect(int x, int y, int w, int h, const uint16_t* data) {
        _lcd->pushImage(x, y, w, h, data);
    }

    /**
     * Renders a 16-bit RGB565 bitmap array inside the off-screen sprite buffer.
     */
    void pushImageBuffer(int x, int y, int w, int h, const uint16_t* data) {
        _sprite->pushImage(x, y, w, h, data);
    }

    /**
     * Renders a bitmap array inside the sprite buffer with a transparent color key.
     */
    void pushImageTransparent(int x, int y, int w, int h, const uint16_t* data, uint16_t transparentColor) {
        _sprite->pushImage(x, y, w, h, data, transparentColor);
    }

    // --- Color Math Primitives ---
    uint16_t color565(uint8_t r, uint8_t g, uint8_t b) {
        return _lcd->color565(r, g, b);
    }

    // Blend two 16-bit RGB565 colors together by ratio (0 = c1, 255 = c2)
    uint16_t blendColor(uint16_t c1, uint16_t c2, uint8_t ratio) {
        uint8_t r1 = (c1 >> 11) & 0x1F;
        uint8_t g1 = (c1 >> 5)  & 0x3F;
        uint8_t b1 =  c1        & 0x1F;

        uint8_t r2 = (c2 >> 11) & 0x1F;
        uint8_t g2 = (c2 >> 5)  & 0x3F;
        uint8_t b2 =  c2        & 0x1F;

        uint8_t r = (r1 * (255 - ratio) + r2 * ratio) >> 8;
        uint8_t g = (g1 * (255 - ratio) + g2 * ratio) >> 8;
        uint8_t b = (b1 * (255 - ratio) + b2 * ratio) >> 8;

        return (r << 11) | (g << 5) | b;
    }

    // --- Advanced Shape & UI Primitives ---

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

    void fillRoundRectDirect(int x, int y, int w, int h, int r, uint16_t color) {
        _lcd->fillRoundRect(x, y, w, h, r, color);
    }

    void drawRoundRectDirect(int x, int y, int w, int h, int r, uint16_t color) {
        _lcd->drawRoundRect(x, y, w, h, r, color);
    }

    void fillTriangleDirect(int x1, int y1, int x2, int y2, int x3, int y3, uint16_t color) {
        _lcd->fillTriangle(x1, y1, x2, y2, x3, y3, color);
    }

    void drawTriangleDirect(int x1, int y1, int x2, int y2, int x3, int y3, uint16_t color) {
        _lcd->drawTriangle(x1, y1, x2, y2, x3, y3, color);
    }

    // Draw 16-bit RGB gradient rectangle
    void drawGradientRectDirect(int x, int y, int w, int h, uint16_t startColor, uint16_t endColor, bool vertical = true) {
        if (vertical) {
            for (int i = 0; i < h; i++) {
                uint8_t ratio = (i * 255) / h;
                uint16_t col = blendColor(startColor, endColor, ratio);
                _lcd->drawFastHLine(x, y + i, w, col);
            }
        } else {
            for (int i = 0; i < w; i++) {
                uint8_t ratio = (i * 255) / w;
                uint16_t col = blendColor(startColor, endColor, ratio);
                _lcd->drawFastVLine(x + i, y, h, col);
            }
        }
    }

    // Draw styled UI Button primitive
    void drawButtonDirect(int x, int y, int w, int h, const char* label, uint16_t btnColor, uint16_t txtColor, uint8_t txtSize = 2) {
        _lcd->fillRoundRect(x, y, w, h, 8, btnColor);
        _lcd->drawRoundRect(x, y, w, h, 8, 0xFFFF);

        int textLen = strlen(label);
        int textW   = textLen * (6 * txtSize);
        int textH   = 8 * txtSize;
        int txtX    = x + (w - textW) / 2;
        int txtY    = y + (h - textH) / 2;

        _lcd->setTextColor(txtColor, btnColor);
        _lcd->setTextSize(txtSize);
        _lcd->setCursor(txtX, txtY);
        _lcd->print(label);
    }

    void drawTextDirect(const char* text, int x, int y, uint16_t color, uint8_t size = 2) {
        _lcd->setTextColor(color, 0x0813);
        _lcd->setTextSize(size);
        _lcd->setCursor(x, y);
        _lcd->print(text);
    }

    // --- Sprite Buffer Primitives ---
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

    void fillRoundRectBuffer(int x, int y, int w, int h, int r, uint16_t color) {
        _sprite->fillRoundRect(x, y, w, h, r, color);
    }

    void pushBuffer() {
        _sprite->pushSprite(_spriteX, _spriteY);
    }
};

#endif // GFX_CONTEXT_H
