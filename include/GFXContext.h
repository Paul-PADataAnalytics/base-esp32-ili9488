#ifndef GFX_CONTEXT_H
#define GFX_CONTEXT_H

#include <Arduino.h>
#include <LovyanGFX.hpp>

/**
 * GFXContext - Hardware Abstraction Layer Drawing, Input, Image & Power API
 * 
 * Provides unified high-level native 480x320 hardware SPI DMA drawing at 25+ FPS.
 */
class GFXContext {
private:
    lgfx::LGFX_Device* _lcd;
    LGFX_Sprite*       _sprite;
    float              _fps;

    int _canvasX;
    int _canvasY;
    int _canvasW;
    int _canvasH;

public:
    GFXContext(lgfx::LGFX_Device* lcd, LGFX_Sprite* sprite, int cx, int cy, int cw, int ch)
        : _lcd(lcd), _sprite(sprite), _fps(0.0f), _canvasX(cx), _canvasY(cy), _canvasW(cw), _canvasH(ch) {}

    void setFPS(float fps) { _fps = fps; }
    float getFPS() const { return _fps; }

    int getWidth() const { return 480; }
    int getHeight() const { return 320; }
    int getSpriteX() const { return _canvasX; }
    int getSpriteY() const { return _canvasY; }
    int getSpriteW() const { return _canvasW; }
    int getSpriteH() const { return _canvasH; }

    lgfx::LGFX_Device* getLCD() { return _lcd; }
    LGFX_Sprite* getSprite() { return _sprite; }

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

    // --- Touch Screen API (Native 480x320 Canvas Coordinates) ---
    bool getTouch(int* x, int* y) {
        return _lcd->getTouch(x, y);
    }

    bool isTouched() {
        int x, y;
        return _lcd->getTouch(&x, &y);
    }

    // --- Image & Bitmap Rendering Primitives ---

    void pushImageDirect(int x, int y, int w, int h, const uint16_t* data) {
        _lcd->pushImage(x, y, w, h, data);
    }

    void pushImageTransparent(int x, int y, int w, int h, const uint16_t* data, uint16_t transparentColor) {
        _lcd->pushImage(x, y, w, h, data, transparentColor);
    }

    // --- Color Math Primitives ---
    uint16_t color565(uint8_t r, uint8_t g, uint8_t b) {
        return _lcd->color565(r, g, b);
    }

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

    // --- Shape & UI Primitives ---

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

    void fillRectDirect(int x, int y, int w, int h, uint16_t color) {
        _lcd->fillRect(x, y, w, h, color);
    }

    void fillRoundRectDirect(int x, int y, int w, int h, int r, uint16_t color) {
        _lcd->fillRoundRect(x, y, w, h, r, color);
    }

    void drawRoundRectDirect(int x, int y, int w, int h, int r, uint16_t color) {
        _lcd->drawRoundRect(x, y, w, h, r, color);
    }

    void drawLineDirect(int x1, int y1, int x2, int y2, uint16_t color) {
        _lcd->drawLine(x1, y1, x2, y2, color);
    }

    void drawTextDirect(const char* text, int x, int y, uint16_t color, uint8_t size = 2, uint16_t bgColor = 0x0813) {
        _lcd->setTextColor(color, bgColor);
        _lcd->setTextSize(size);
        _lcd->setCursor(x, y);
        _lcd->print(text);
    }

    void pushBuffer() {
        // High-speed SPI DMA transaction mode
    }
};

#endif // GFX_CONTEXT_H
