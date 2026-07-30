#ifndef ANIMATION_APP_H
#define ANIMATION_APP_H

#include "BaseApp.h"
#include "GFXContext.h"
#include "SleepManager.h"
#include "SampleIcons.h"
#include "TileSet.h"
#include "TileMap.h"
#include "LayerManager.h"
#include "RetroAssets.h"
#include "RetroSprite.h"
#include "ParticleEngine.h"
#include "VirtualGamepad.h"
#include "Camera2D.h"
#include "SoundEngine.h"
#include "Physics2D.h"
#include "SaveSystem.h"
#include "UIManager.h"
#include <Arduino.h>

RTC_DATA_ATTR static int rtcWakeupCount = 0;

/**
 * HAL Framework & Retro Engine v1.4.3 (Touch X-Axis Un-Mirrored Calibration)
 */
class AnimationApp : public BaseApp {

private:
    struct Ball {
        float x, y;
        float vx, vy;
        float radius;
        uint16_t color;
    };

    SleepManager   _sleepManager;
    TileSet        _tileSet;
    TileMap        _tileMap;
    LayerManager   _layerManager;
    ParticleEngine _particleEngine;
    VirtualGamepad _gamepad;
    Camera2D       _camera;
    SoundEngine    _sound;
    SaveSystem     _saveSystem;
    UIManager      _uiManager;

    RetroSprite    _heroSprite;
    AnimatedSprite _coinSprite;
    Ball           _balls[3];

    float _bgRotation;
    float _bgScrollX;
    float _heroScale;
    float _scaleDirection;
    int   _highScore;
    int   _score;

    int   _activeTouchX;
    int   _activeTouchY;
    bool  _isTouchActive;

public:
    AnimationApp() 
        : _sleepManager(20, SleepMode::DEEP_SLEEP),
          _tileSet(RETRO_TILESET_32x32, 16, 16, 32, 32),
          _tileMap(&_tileSet, RETRO_LEVEL_MAP, 16, 10),
          _particleEngine(150),
          _camera(240, 160),
          _sound(25),
          _saveSystem("hal_game"),
          _heroSprite(ICON_POWER_16x16, 16, 16, 0x0000),
          _coinSprite(RETRO_TILESET_32x32, 16, 16, 4, 8.0f, 0x0000),
          _bgRotation(0.0f), _bgScrollX(0.0f),
          _heroScale(2.0f), _scaleDirection(1.0f),
          _highScore(0), _score(0),
          _activeTouchX(0), _activeTouchY(0), _isTouchActive(false) {}

    void setup(GFXContext& gfx) override {
        _sound.init();
        _saveSystem.begin();
        _highScore = _saveSystem.getHighScore();

        _sleepManager.onPreSleep([this]() {
            Serial.println("[HAL ENGINE] Saving high score before sleep...");
            _saveSystem.saveHighScore(_score);
        });

        _sleepManager.onPostWake([this]() {
            rtcWakeupCount++;
            _sound.playPowerup();
            Serial.printf("[HAL ENGINE] Restored state after wake! Wakeups: %d\n", rtcWakeupCount);
        });

        _sleepManager.checkAndNotifyDeepSleepWakeup();

        // Configure Hero & Coin Sprites
        _heroSprite.setPosition(180, 140);
        _heroSprite.setScale(2.0f, 2.0f);
        _heroSprite.setPivot(8, 8);

        _coinSprite.setPosition(280, 100);
        _coinSprite.setScale(1.8f, 1.8f);
        _coinSprite.setPivot(8, 8);

        // Configure Energy Spheres
        uint16_t sphereColors[3] = { gfx.color565(0, 255, 255), gfx.color565(255, 255, 0), gfx.color565(255, 0, 255) };
        for (int i = 0; i < 3; i++) {
            _balls[i].x = random(40, 440);
            _balls[i].y = random(60, 260);
            _balls[i].vx = random(2, 4) * (random(0, 2) ? 1 : -1);
            _balls[i].vy = random(2, 4) * (random(0, 2) ? 1 : -1);
            _balls[i].radius = random(10, 14);
            _balls[i].color = sphereColors[i];
        }

        // Configure 2D Camera
        _camera.setPosition(240, 160);
        _camera.setWorldBounds(100, 100, 380, 220);

        _uiManager.showToast("Touch Un-Mirrored Calibrated!", 0x07FF, 3.0f);

        // --- Full-Screen 480x320 Multi-Layer Pipeline ---

        // 1. BACKGROUND LAYER (Renders at lowest depth below everything)
        _layerManager.addLayer("Background", LayerRole::BACKGROUND, 0, [this](GFXContext& gfx, LGFX_Sprite* buffer, Layer& layer) {
            gfx.fillScreen(gfx.color565(6, 14, 25));
            for (int i = 0; i < 8; i++) {
                float ang = (layer.getRotation() + i * 45) * DEG_TO_RAD;
                int x2 = (int)(_camera.getRenderX() + cos(ang) * 160);
                int y2 = (int)(_camera.getRenderY() + sin(ang) * 160);
                gfx.drawLineDirect((int)_camera.getRenderX(), (int)_camera.getRenderY(), x2, y2, gfx.color565(20, 40, 70));
            }
        });

        // 2. WORLD MAP LAYER (TileMap grid level geometry)
        _layerManager.addLayer("TileMap", LayerRole::WORLD_MAP, 0, [this](GFXContext& gfx, LGFX_Sprite* buffer, Layer& layer) {
            _tileMap.render(gfx.getLCD(), (int)layer.getTranslationX(), 0, 0x0000);
        });

        // 3. ENTITIES LAYER (Hero, Coins, Energy Spheres, Particles)
        _layerManager.addLayer("Entities", LayerRole::ENTITIES, 0, [this](GFXContext& gfx, LGFX_Sprite* buffer, Layer& layer) {
            // Energy Spheres
            for (int i = 0; i < 3; i++) {
                gfx.drawCircleDirect((int)_balls[i].x, (int)_balls[i].y, (int)_balls[i].radius, _balls[i].color, true);
                gfx.drawCircleDirect((int)_balls[i].x, (int)_balls[i].y, (int)_balls[i].radius + 1, 0xFFFF, false);
            }

            _heroSprite.render(gfx.getLCD());
            _coinSprite.render(gfx.getLCD());
            _particleEngine.render(gfx.getLCD());
        });

        // 4. FOREGROUND LAYER (Tree canopy occlusion above entities!)
        _layerManager.addLayer("Foreground", LayerRole::FOREGROUND, 0, [this](GFXContext& gfx, LGFX_Sprite* buffer, Layer& layer) {
            _tileSet.drawTile(gfx.getLCD(), 3, 80 + (int)layer.getTranslationX(), 20, 0x0000);
            _tileSet.drawTile(gfx.getLCD(), 3, 176 + (int)layer.getTranslationX(), 20, 0x0000);
        });

        // 5. UI OVERLAY LAYER (HUD Bar, Score, Sleep Countdown)
        _layerManager.addLayer("UIOverlay", LayerRole::UI_OVERLAY, 0, [this](GFXContext& gfx, LGFX_Sprite* buffer, Layer& layer) {
            // Top HUD Bar
            gfx.fillRectDirect(0, 0, 480, 28, gfx.color565(12, 24, 45));
            gfx.drawRectDirect(0, 0, 480, 28, 0x07FF);

            gfx.drawTextDirect("HAL v1.4", 15, 6, gfx.color565(0, 255, 255), 2, gfx.color565(12, 24, 45));

            char scoreBuf[32];
            snprintf(scoreBuf, sizeof(scoreBuf), "Score:%d High:%d", _score, _highScore);
            gfx.drawTextDirect(scoreBuf, 150, 6, gfx.color565(0, 255, 120), 2, gfx.color565(12, 24, 45));

            uint32_t secRemaining = _sleepManager.getInactivitySecondsRemaining();
            char sleepBuf[32];
            snprintf(sleepBuf, sizeof(sleepBuf), "Sleep:%lds ", (long)secRemaining);
            gfx.drawTextDirect(sleepBuf, 370, 6, gfx.color565(255, 165, 0), 2, gfx.color565(12, 24, 45));

            if (rtcWakeupCount > 0) {
                char wakeBuf[32];
                snprintf(wakeBuf, sizeof(wakeBuf), "Wakes:%d", rtcWakeupCount);
                gfx.drawTextDirect(wakeBuf, 15, 295, gfx.color565(0, 255, 120), 2, 0x0000);
            }

            if (_isTouchActive) {
                char touchBuf[32];
                snprintf(touchBuf, sizeof(touchBuf), "Touch:(%d,%d)", _activeTouchX, _activeTouchY);
                gfx.drawTextDirect(touchBuf, 180, 295, gfx.color565(255, 255, 0), 2, 0x0000);
            }
        });

        _sleepManager.resetInactivityTimer();
    }

    void update(float deltaTime) override {
        // 1. Background layer rotation & parallax translation
        Layer* bgLayer  = _layerManager.getLayer("Background");
        Layer* mapLayer = _layerManager.getLayer("TileMap");
        Layer* fgLayer  = _layerManager.getLayer("Foreground");

        if (bgLayer) bgLayer->rotate(15.0f * deltaTime);
        
        _bgScrollX -= 8.0f * deltaTime;
        if (_bgScrollX <= -32.0f) _bgScrollX += 32.0f;

        if (mapLayer) mapLayer->setTranslation(_bgScrollX, 0);
        if (fgLayer)  fgLayer->setTranslation(_bgScrollX * 1.5f, 0);

        // 2. Hero Sprite Pulsing Scale & Rotation
        _heroSprite.rotate(90.0f * deltaTime);
        _heroScale += 0.5f * _scaleDirection * deltaTime;
        if (_heroScale >= 2.5f || _heroScale <= 1.2f) {
            _scaleDirection *= -1.0f;
        }
        _heroSprite.setScale(_heroScale, _heroScale);

        // 3. Update Energy Spheres physics
        for (int i = 0; i < 3; i++) {
            _balls[i].x += _balls[i].vx;
            _balls[i].y += _balls[i].vy;

            if (_balls[i].x - _balls[i].radius <= 10 || _balls[i].x + _balls[i].radius >= 470) {
                _balls[i].vx *= -1;
            }
            if (_balls[i].y - _balls[i].radius <= 35 || _balls[i].y + _balls[i].radius >= 290) {
                _balls[i].vy *= -1;
            }
        }

        // 4. Smooth Camera Tracking on Hero Sprite
        _camera.setTarget(_heroSprite.getX(), _heroSprite.getY());
        _camera.update(deltaTime);

        // 5. Update Animations, Particles, & UI Manager
        _coinSprite.update(deltaTime);
        _particleEngine.update(deltaTime);
        _uiManager.update(deltaTime);

        // 6. Physics Collision Check (Hero & Coin)
        Circle2D heroCircle = {_heroSprite.getX(), _heroSprite.getY(), 12.0f};
        Circle2D coinCircle = {_coinSprite.getX(), _coinSprite.getY(), 14.0f};
        if (Physics2D::checkCircleCollision(heroCircle, coinCircle)) {
            _score += 100;
            if (_score > _highScore) _highScore = _score;
            _sound.playCoin();
            _camera.triggerShake(6.0f, 0.3f);
            _particleEngine.emitExplosion(_coinSprite.getX(), _coinSprite.getY(), 16, 0xFFE0, 0x07E0);
            _coinSprite.setPosition(random(80, 400), random(80, 240));
        }
    }

    void render(GFXContext& gfx) override {
        // Process Touch & Gamepad Controls
        _gamepad.update(gfx);
        const auto& btn = _gamepad.getState();

        if (btn.left)  _heroSprite.move(-3.0f, 0);
        if (btn.right) _heroSprite.move(3.0f, 0);
        if (btn.up)    _heroSprite.move(0, -3.0f);
        if (btn.down)  _heroSprite.move(0, 3.0f);

        if (btn.btnA) {
            _particleEngine.emitExplosion(_heroSprite.getX(), _heroSprite.getY(), 12, 0xFFE0, 0xF800);
            _sound.playLaser();
            _camera.triggerShake(10.0f, 0.4f);
            _sleepManager.resetInactivityTimer();
        }

        _isTouchActive = gfx.getTouch(&_activeTouchX, &_activeTouchY);
        if (_isTouchActive) {
            _sleepManager.resetInactivityTimer();
        }

        // Check Auto-Sleep Trigger
        if (_sleepManager.shouldAutoSleep()) {
            _sleepManager.triggerSleep(gfx);
            return;
        }

        // Composite ALL Layers directly across full 480x320 screen canvas
        _layerManager.renderAll(gfx, nullptr);

        // Render On-Screen Touch Gamepad Controls & Toast Banner across 480x320 screen
        _gamepad.render(gfx);
        _uiManager.renderToast(gfx);
    }
};

#endif // ANIMATION_APP_H
