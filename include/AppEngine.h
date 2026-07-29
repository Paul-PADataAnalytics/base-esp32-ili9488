#ifndef APP_ENGINE_H
#define APP_ENGINE_H

#include "DisplayDriver.h"
#include "BaseApp.h"

/**
 * High-Level Hardware Abstraction Application Engine
 * 
 * Completely decouples all LCD driver, SPI DMA transfers, and hardware management
 * from user applications.
 */
class AppEngine {

private:
    LGFX_Sprite   _sprite;
    BaseApp*      _currentApp;

    unsigned long _lastFrameTime;
    unsigned long _lastFPSTime;
    int           _frameCount;
    float         _currentFPS;

    int _width;
    int _height;
    int _spriteWidth;
    int _spriteHeight;
    int _spriteX;
    int _spriteY;

public:
    AppEngine() 
        : _sprite(&tft_driver),
          _currentApp(nullptr),
          _lastFrameTime(0),
          _lastFPSTime(0),
          _frameCount(0),
          _currentFPS(0.0f),
          _width(480),
          _height(320),
          _spriteWidth(180),
          _spriteHeight(180),
          _spriteX(150),
          _spriteY(80) {}

    // Initialize display hardware & double-buffered sprite
    bool init(int spriteW = 180, int spriteH = 180) {
        _spriteWidth  = spriteW;
        _spriteHeight = spriteH;
        _spriteX      = (_width - _spriteWidth) / 2;
        _spriteY      = (_height - _spriteHeight) / 2 + 10;

        pinMode(32, OUTPUT);
        digitalWrite(32, HIGH); // Backlight HIGH

        tft_driver.init();
        tft_driver.setBrightness(255);
        tft_driver.setRotation(1); // Landscape mode (480x320)
        tft_driver.fillScreen(0x0813); // Dark cyber space background

        _sprite.setColorDepth(16); // 16-bit RGB565
        void* ptr = _sprite.createSprite(_spriteWidth, _spriteHeight);

        _lastFrameTime = millis();
        _lastFPSTime   = millis();

        return (ptr != nullptr);
    }

    // Set active application to run
    void setApp(BaseApp* app) {
        _currentApp = app;
        if (_currentApp) {
            _currentApp->setup();
        }
    }

    // Engine loop execution call
    void tick() {
        if (!_currentApp) return;

        unsigned long now = millis();
        float deltaTime = (now - _lastFrameTime) / 1000.0f;
        if (deltaTime <= 0.0f) deltaTime = 0.016f;
        _lastFrameTime = now;

        // 1. App state/physics update
        _currentApp->update(deltaTime);

        // 2. App rendering pass
        tft_driver.startWrite();
        _currentApp->render(*this);
        tft_driver.endWrite();

        // 3. FPS calculation
        _frameCount++;
        if (now - _lastFPSTime >= 1000) {
            _currentFPS = _frameCount * 1000.0f / (now - _lastFPSTime);
            _lastFPSTime = now;
            _frameCount = 0;
            Serial.printf("[FRAMEWORK ENGINE] Running - FPS: %.1f\n", _currentFPS);
        }
    }

    // --- High-Level Drawing API (Zero Hardware Details Required) ---

    int getWidth() const { return _width; }
    int getHeight() const { return _height; }
    float getFPS() const { return _currentFPS; }
    uint16_t color565(uint8_t r, uint8_t g, uint8_t b) { return tft_driver.color565(r, g, b); }

    int getSpriteX() const { return _spriteX; }
    int getSpriteY() const { return _spriteY; }
    int getSpriteWidth() const { return _spriteWidth; }
    int getSpriteHeight() const { return _spriteHeight; }

    bool overlapsSprite(float x, float y, float radius) const {
        return (x + radius + 4 >= _spriteX &&
                x - radius - 4 <= _spriteX + _spriteWidth &&
                y + radius + 4 >= _spriteY &&
                y - radius - 4 <= _spriteY + _spriteHeight);
    }

    void clearScreen(uint16_t color = 0x0813) {
        tft_driver.fillScreen(color);
    }

    void clearSpriteBuffer(uint16_t color = 0x0813) {
        _sprite.fillScreen(color);
    }

    void drawCircleDirect(int x, int y, int r, uint16_t color, bool fill = true) {
        if (fill) {
            tft_driver.fillCircle(x, y, r, color);
        } else {
            tft_driver.drawCircle(x, y, r, color);
        }
    }

    void eraseCircleDirect(int x, int y, int r, uint16_t color = 0x0813) {
        tft_driver.fillCircle(x, y, r + 2, color);
    }

    void drawRectDirect(int x, int y, int w, int h, uint16_t color) {
        tft_driver.drawRect(x, y, w, h, color);
    }

    void drawTextDirect(const char* text, int x, int y, uint16_t color, uint8_t size = 2) {
        tft_driver.setTextColor(color, 0x0813);
        tft_driver.setTextSize(size);
        tft_driver.setCursor(x, y);
        tft_driver.print(text);
    }

    void drawLineSprite(int x1, int y1, int x2, int y2, uint16_t color) {
        _sprite.drawLine(x1, y1, x2, y2, color);
    }

    void drawCircleSprite(int x, int y, int r, uint16_t color, bool fill = true) {
        if (fill) {
            _sprite.fillCircle(x, y, r, color);
        } else {
            _sprite.drawCircle(x, y, r, color);
        }
    }

    void pushSpriteBuffer() {
        _sprite.pushSprite(_spriteX, _spriteY);
    }
};

#endif // APP_ENGINE_H
