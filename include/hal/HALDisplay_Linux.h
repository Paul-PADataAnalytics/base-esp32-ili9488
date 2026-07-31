#ifndef HAL_DISPLAY_LINUX_H
#define HAL_DISPLAY_LINUX_H

#if defined(PLATFORM_LINUX) || !defined(ARDUINO)

#include "HALDisplay.h"
#include <SDL2/SDL.h>
#include <cstdio>

/**
 * HALDisplay_Linux — Native SDL2 Desktop Simulator HAL
 *
 * Runs the ESP32 game engine inside a native 960x640 desktop window (2x scaled 480x320 canvas).
 * Maps mouse clicks and drags to touch coordinates.
 */
class HALDisplay_Linux : public HALDisplay {
private:
    SDL_Window*   _window   = nullptr;
    SDL_Renderer* _renderer = nullptr;
    SDL_Texture*  _texture  = nullptr;

    uint16_t* _framebuffer = nullptr;
    int _width  = 480;
    int _height = 320;
    int _scale  = 2; // 2x window scaling (960x640)

    bool _running    = false;
    bool _isTouched  = false;
    int  _touchX     = 0;
    int  _touchY     = 0;

public:
    HALDisplay_Linux(int width = 480, int height = 320, int scale = 2)
        : _width(width), _height(height), _scale(scale) {}

    ~HALDisplay_Linux() override {
        cleanup();
    }

    bool init() override {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
            printf("[HAL LINUX ERROR] SDL_Init failed: %s\n", SDL_GetError());
            return false;
        }

        _window = SDL_CreateWindow(
            "ESP32 ILI9488 Game Engine & UI Framework (Linux Native Desktop Simulator)",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            _width * _scale, _height * _scale,
            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
        );

        if (!_window) {
            printf("[HAL LINUX ERROR] SDL_CreateWindow failed: %s\n", SDL_GetError());
            return false;
        }

        _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (!_renderer) {
            _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_SOFTWARE);
        }

        _texture = SDL_CreateTexture(
            _renderer,
            SDL_PIXELFORMAT_RGB565,
            SDL_TEXTUREACCESS_STREAMING,
            _width, _height
        );

        _framebuffer = new uint16_t[_width * _height];
        memset(_framebuffer, 0, _width * _height * sizeof(uint16_t));

        _running = true;
        printf("[HAL LINUX] Native SDL2 Desktop Window initialized (%dx%d canvas, %dx scaling).\n",
               _width, _height, _scale);
        return true;
    }

    void setRotation(uint8_t rotation) override {
        // Landscape 480x320 is default
    }

    void fillScreen(uint16_t color) override {
        if (!_framebuffer) return;
        for (int i = 0; i < _width * _height; i++) {
            _framebuffer[i] = color;
        }
        present();
    }

    bool getTouch(int* x, int* y) override {
        if (x) *x = _touchX;
        if (y) *y = _touchY;
        return _isTouched;
    }

    bool isTouched() override {
        return _isTouched;
    }

    void pollEvents() override {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    _running = false;
                    break;

                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        _isTouched = true;
                        updateTouchPos(event.button.x, event.button.y);
                    }
                    break;

                case SDL_MOUSEBUTTONUP:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        _isTouched = false;
                    }
                    break;

                case SDL_MOUSEMOTION:
                    if (_isTouched) {
                        updateTouchPos(event.motion.x, event.motion.y);
                    }
                    break;

                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        _running = false;
                    }
                    break;
            }
        }
    }

    bool isRunning() const override {
        return _running;
    }

    uint16_t color565(uint8_t r, uint8_t g, uint8_t b) override {
        return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    }

    void setBacklight(bool on) override {}
    void sleep() override {}
    void wakeup() override {}

    /** Update simulator screen from full or band framebuffer. */
    void updateFramebuffer(int x, int y, int w, int h, const uint16_t* pixels) {
        if (!_framebuffer || !pixels) return;
        for (int row = 0; row < h; row++) {
            int destY = y + row;
            if (destY >= 0 && destY < _height) {
                memcpy(&_framebuffer[destY * _width + x], &pixels[row * w], w * sizeof(uint16_t));
            }
        }
        present();
    }

private:
    void updateTouchPos(int winX, int winY) {
        int winW, winH;
        SDL_GetWindowSize(_window, &winW, &winH);
        _touchX = (int)((float)winX / winW * _width);
        _touchY = (int)((float)winY / winH * _height);
        if (_touchX < 0) _touchX = 0;
        if (_touchX >= _width) _touchX = _width - 1;
        if (_touchY < 0) _touchY = 0;
        if (_touchY >= _height) _touchY = _height - 1;
    }

    void present() {
        if (!_renderer || !_texture || !_framebuffer) return;
        SDL_UpdateTexture(_texture, nullptr, _framebuffer, _width * sizeof(uint16_t));
        SDL_RenderClear(_renderer);
        SDL_RenderCopy(_renderer, _texture, nullptr, nullptr);
        SDL_RenderPresent(_renderer);
    }

    void cleanup() {
        if (_framebuffer) { delete[] _framebuffer; _framebuffer = nullptr; }
        if (_texture) { SDL_DestroyTexture(_texture); _texture = nullptr; }
        if (_renderer) { SDL_DestroyRenderer(_renderer); _renderer = nullptr; }
        if (_window) { SDL_DestroyWindow(_window); _window = nullptr; }
        SDL_Quit();
    }
};

#endif // PLATFORM_LINUX
#endif // HAL_DISPLAY_LINUX_H
