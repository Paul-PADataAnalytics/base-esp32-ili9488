#ifndef CAMERA_2D_H
#define CAMERA_2D_H

#include <Arduino.h>

/**
 * Camera2D - 2D Game Camera Engine with Damping & Screen Shake
 * 
 * Tracks target sprites smoothly, enforces world boundaries,
 * and applies screen shake fx for explosions and impacts.
 */
class Camera2D {
private:
    float _x, _y;
    float _targetX, _targetY;
    float _lerpSpeed; // 0.0f to 1.0f

    // Screen Shake
    float _shakeIntensity;
    float _shakeDuration;
    float _shakeTimer;
    float _offsetX, _offsetY;

    // World Bounds
    float _minX, _minY;
    float _maxX, _maxY;
    bool  _hasBounds;

public:
    Camera2D(float startX = 0, float startY = 0)
        : _x(startX), _y(startY), _targetX(startX), _targetY(startY),
          _lerpSpeed(0.1f), _shakeIntensity(0.0f), _shakeDuration(0.0f), _shakeTimer(0.0f),
          _offsetX(0.0f), _offsetY(0.0f), _minX(0), _minY(0), _maxX(0), _maxY(0), _hasBounds(false) {}

    void setPosition(float x, float y) { _x = x; _y = y; }
    void setTarget(float tx, float ty) { _targetX = tx; _targetY = ty; }
    void setLerpSpeed(float speed) { _lerpSpeed = speed; }

    void setWorldBounds(float minX, float minY, float maxX, float maxY) {
        _minX = minX; _minY = minY;
        _maxX = maxX; _maxY = maxY;
        _hasBounds = true;
    }

    void triggerShake(float intensity = 8.0f, float duration = 0.4f) {
        _shakeIntensity = intensity;
        _shakeDuration  = duration;
        _shakeTimer     = duration;
    }

    void update(float deltaTime) {
        // 1. Smooth lerp position tracking
        _x += (_targetX - _x) * _lerpSpeed;
        _y += (_targetY - _y) * _lerpSpeed;

        // Enforce world bounds
        if (_hasBounds) {
            if (_x < _minX) _x = _minX;
            if (_x > _maxX) _x = _maxX;
            if (_y < _minY) _y = _minY;
            if (_y > _maxY) _y = _maxY;
        }

        // 2. Process Screen Shake
        if (_shakeTimer > 0.0f) {
            _shakeTimer -= deltaTime;
            float factor = _shakeTimer / _shakeDuration;
            _offsetX = random(-_shakeIntensity, _shakeIntensity) * factor;
            _offsetY = random(-_shakeIntensity, _shakeIntensity) * factor;
        } else {
            _offsetX = 0.0f;
            _offsetY = 0.0f;
        }
    }

    float getRenderX() const { return _x + _offsetX; }
    float getRenderY() const { return _y + _offsetY; }
    float getX() const { return _x; }
    float getY() const { return _y; }
};

#endif // CAMERA_2D_H
