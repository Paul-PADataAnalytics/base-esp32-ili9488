#ifndef RETRO_SPRITE_H
#define RETRO_SPRITE_H

#include <Arduino.h>
#include <LovyanGFX.hpp>

/**
 * RetroSprite - Advanced 2D Sprite Module
 */
class RetroSprite {
protected:
    const uint16_t* _bitmap;
    int   _width;
    int   _height;
    float _x, _y;
    float _scaleX, _scaleY;
    float _angleDeg;
    float _skewX, _skewY;
    float _pivotX, _pivotY;
    uint16_t _transparentKey;

public:
    RetroSprite(const uint16_t* bitmap, int w, int h, uint16_t transparentKey = 0x0000)
        : _bitmap(bitmap), _width(w), _height(h),
          _x(0.0f), _y(0.0f),
          _scaleX(1.0f), _scaleY(1.0f),
          _angleDeg(0.0f),
          _skewX(0.0f), _skewY(0.0f),
          _pivotX(w / 2.0f), _pivotY(h / 2.0f),
          _transparentKey(transparentKey) {}

    virtual ~RetroSprite() {}

    void setPosition(float x, float y) { _x = x; _y = y; }
    void move(float dx, float dy) { _x += dx; _y += dy; }
    void setScale(float sx, float sy) { _scaleX = sx; _scaleY = sy; }
    void setRotation(float angleDeg) { _angleDeg = angleDeg; }
    void rotate(float deltaDeg) { _angleDeg += deltaDeg; }
    void setSkew(float kx, float ky) { _skewX = kx; _skewY = ky; }
    void setPivot(float px, float py) { _pivotX = px; _pivotY = py; }
    void setTransparentKey(uint16_t key) { _transparentKey = key; }

    float getX() const { return _x; }
    float getY() const { return _y; }
    float getScaleX() const { return _scaleX; }
    float getScaleY() const { return _scaleY; }
    float getRotation() const { return _angleDeg; }
    int getWidth() const { return _width; }
    int getHeight() const { return _height; }

    virtual void render(LovyanGFX* canvas, int bandY = 0) {
        if (!_bitmap || !canvas) return;

        float localY = _y - bandY;

        LGFX_Sprite srcSprite;
        srcSprite.setColorDepth(16);
        srcSprite.setBuffer((uint8_t*)_bitmap, _width, _height, 16);
        srcSprite.setPivot(_pivotX, _pivotY);

        srcSprite.pushRotateZoom(
            canvas,
            _x, localY,
            _angleDeg,
            _scaleX, _scaleY,
            _transparentKey
        );
    }
};

/**
 * AnimatedSprite - Multi-Frame Animated Sprite Engine
 */
class AnimatedSprite : public RetroSprite {
private:
    int   _frameWidth;
    int   _frameHeight;
    int   _totalFrames;
    int   _currentFrame;
    float _frameDuration;
    float _elapsedTime;
    bool  _loop;
    bool  _playing;

public:
    AnimatedSprite(const uint16_t* spriteSheet, int frameW, int frameH, int totalFrames, float fps = 10.0f, uint16_t transparentKey = 0x0000)
        : RetroSprite(spriteSheet, frameW, frameH, transparentKey),
          _frameWidth(frameW), _frameHeight(frameH),
          _totalFrames(totalFrames), _currentFrame(0),
          _frameDuration(1.0f / fps), _elapsedTime(0.0f),
          _loop(true), _playing(true) {}

    void setFPS(float fps) {
        if (fps > 0.0f) _frameDuration = 1.0f / fps;
    }

    void setLoop(bool loop) { _loop = loop; }
    void play() { _playing = true; }
    void pause() { _playing = false; }
    void setFrame(int frame) { _currentFrame = frame % _totalFrames; }
    int getCurrentFrame() const { return _currentFrame; }

    void update(float deltaTime) {
        if (!_playing || _totalFrames <= 1) return;

        _elapsedTime += deltaTime;
        if (_elapsedTime >= _frameDuration) {
            _elapsedTime -= _frameDuration;
            _currentFrame++;
            if (_currentFrame >= _totalFrames) {
                if (_loop) _currentFrame = 0;
                else {
                    _currentFrame = _totalFrames - 1;
                    _playing = false;
                }
            }
        }
    }

    void render(LovyanGFX* canvas, int bandY = 0) override {
        if (!_bitmap || !canvas) return;

        float localY = _y - bandY;
        const uint16_t* framePointer = _bitmap + (_currentFrame * _frameWidth * _frameHeight);

        LGFX_Sprite srcSprite;
        srcSprite.setColorDepth(16);
        srcSprite.setBuffer((uint8_t*)framePointer, _frameWidth, _frameHeight, 16);
        srcSprite.setPivot(_pivotX, _pivotY);

        srcSprite.pushRotateZoom(
            canvas,
            _x, localY,
            _angleDeg,
            _scaleX, _scaleY,
            _transparentKey
        );
    }
};

#endif // RETRO_SPRITE_H
