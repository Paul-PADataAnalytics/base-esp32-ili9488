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
 * HAL Framework & Retro Engine v1.4.7 (Guaranteed 16-Bit RGB565 Double-Buffered Engine)
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
        : _sleepManager(0, SleepMode::LIGHT_SLEEP),
          _tileSet(RETRO_TILESET_32x32, 16, 16, 32, 32),
          _tileMap(&_tileSet, RETRO_LEVEL_MAP, 16, 10),
          _particleEngine(150),
          _camera(160, 80),
          _sound(25),
          _saveSystem("hal_game"),
          _heroSprite(ICON_POWER_16x16, 16, 16, 0x0000),
          _coinSprite(RETRO_TILESET_32x32, 16, 16, 4, 8.0f, 0x0000),
          _bgRotation(0.0f), _bgScrollX(0.0f),
          _heroScale(1.2f), _scaleDirection(1.0f),
          _highScore(0), _score(0),
          _activeTouchX(0), _activeTouchY(0), _isTouchActive(false) {}

    void setup(GFXContext& gfx) override {
        _sound.init();
        _saveSystem.begin();
        _highScore = _saveSystem.getHighScore();

        _sleepManager.disableAutoSleep();

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
        _heroSprite.setPosition(120, 70);
        _heroSprite.setScale(1.2f, 1.2f);
        _heroSprite.setPivot(8, 8);

        _coinSprite.setPosition(180, 50);
        _coinSprite.setScale(1.2f, 1.2f);
        _coinSprite.setPivot(8, 8);

        // Configure Energy Spheres
        uint16_t sphereColors[3] = { gfx.color565(0, 255, 255), gfx.color565(255, 255, 0), gfx.color565(255, 0, 255) };
        for (int i = 0; i < 3; i++) {
            _balls[i].x = random(20, 300);
            _balls[i].y = random(30, 130);
            _balls[i].vx = random(2, 4) * (random(0, 2) ? 1 : -1);
            _balls[i].vy = random(2, 4) * (random(0, 2) ? 1 : -1);
            _balls[i].radius = random(6, 10);
            _balls[i].color = sphereColors[i];
        }

        // Configure 2D Camera
        _camera.setPosition(160, 80);
        _camera.setWorldBounds(40, 40, 280, 140);

        _uiManager.showToast("Guaranteed 16-Bit Double-Buffered!", 0x07FF, 3.5f);

        // --- 16-Bit RGB565 Double-Buffered Layer Pipeline ---

        // 1. BACKGROUND LAYER (Renders at lowest depth below everything)
        _layerManager.addLayer("Background", LayerRole::BACKGROUND, 0, [this](GFXContext& gfx, LGFX_Sprite* buffer, Layer& layer) {
            gfx.fillScreen(gfx.color565(6, 14, 25));
            for (int i = 0; i < 8; i++) {
                float ang = (layer.getRotation() + i * 45) * DEG_TO_RAD;
                int x2 = (int)(_camera.getRenderX() + cos(ang) * 90);
                int y2 = (int)(_camera.getRenderY() + sin(ang) * 90);
                gfx.drawLineDirect((int)_camera.getRenderX(), (int)_camera.getRenderY(), x2, y2, gfx.color565(20, 40, 70));
            }
        });

        // 2. WORLD MAP LAYER (TileMap grid level geometry)
        _layerManager.addLayer("TileMap", LayerRole::WORLD_MAP, 0, [this](GFXContext& gfx, LGFX_Sprite* buffer, Layer& layer) {
            _tileMap.render(gfx.getSprite(), (int)layer.getTranslationX(), 0, 0x0000);
        });

        // 3. ENTITIES LAYER (Hero, Coins, Energy Spheres, Particles)
        _layerManager.addLayer("Entities", LayerRole::ENTITIES, 0, [this](GFXContext& gfx, LGFX_Sprite* buffer, Layer& layer) {
            // Energy Spheres
            for (int i = 0; i < 3; i++) {
                gfx.drawCircleDirect((int)_balls[i].x, (int)_balls[i].y, (int)_balls[i].radius, _balls[i].color, true);
                gfx.drawCircleDirect((int)_balls[i].x, (int)_balls[i].y, (int)_balls[i].radius + 1, 0xFFFF, false);
            }

            _heroSprite.render(gfx.getSprite());
            _coinSprite.render(gfx.getSprite());
            _particleEngine.render(gfx.getSprite());
        });

        // 4. FOREGROUND LAYER (Tree canopy occlusion above entities!)
        _layerManager.addLayer("Foreground", LayerRole::FOREGROUND, 0, [this](GFXContext& gfx, LGFX_Sprite* buffer, Layer& layer) {
            _tileSet.drawTile(gfx.getSprite(), 3, 40 + (int)layer.getTranslationX(), 15, 0x0000);
            _tileSet.drawTile(gfx.getSprite(), 3, 110 + (int)layer.getTranslationX(), 15, 0x0000);
        });

        // 5. UI OVERLAY LAYER (HUD Bar, Score, Sleep Countdown)
        _layerManager.addLayer("UIOverlay", LayerRole::UI_OVERLAY, 0, [this](GFXContext& gfx, LGFX_Sprite* buffer, Layer& layer) {
            // Top HUD Bar
            gfx.fillRectDirect(0, 0, 320, 18, gfx.color565(12, 24, 45));
            gfx.drawRectDirect(0, 0, 320, 18, 0x07FF);

            gfx.drawTextDirect("HAL v1.4", 4, 2, gfx.color565(0, 255, 255), 1, gfx.color565(12, 24, 45));

            char scoreBuf[32];
            snprintf(scoreBuf, sizeof(scoreBuf), "Score:%d High:%d", _score, _highScore);
            gfx.drawTextDirect(scoreBuf, 90, 2, gfx.color565(0, 255, 120), 1, gfx.color565(12, 24, 45));

            gfx.drawTextDirect("Sleep:OFF", 230, 2, gfx.color565(255, 165, 0), 1, gfx.color565(12, 24, 45));

            if (rtcWakeupCount > 0) {
                char wakeBuf[32];
                snprintf(wakeBuf, sizeof(wakeBuf), "Wakes:%d", rtcWakeupCount);
                gfx.drawTextDirect(wakeBuf, 4, 148, gfx.color565(0, 255, 120), 1, 0x0000);
            }

            if (_isTouchActive) {
                char touchBuf[32];
                snprintf(touchBuf, sizeof(touchBuf), "Touch:(%d,%d)", _activeTouchX, _activeTouchY);
                gfx.drawTextDirect(touchBuf, 110, 148, gfx.color565(255, 255, 0), 1, 0x0000);
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
        if (_heroScale >= 1.6f || _heroScale <= 0.9f) {
            _scaleDirection *= -1.0f;
        }
        _heroSprite.setScale(_heroScale, _heroScale);

        // 3. Update Energy Spheres physics
        for (int i = 0; i < 3; i++) {
            _balls[i].x += _balls[i].vx;
            _balls[i].y += _balls[i].vy;

            if (_balls[i].x - _balls[i].radius <= 5 || _balls[i].x + _balls[i].radius >= 315) {
                _balls[i].vx *= -1;
            }
            if (_balls[i].y - _balls[i].radius <= 20 || _balls[i].y + _balls[i].radius >= 155) {
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
        Circle2D heroCircle = {_heroSprite.getX(), _heroSprite.getY(), 10.0f};
        Circle2D coinCircle = {_coinSprite.getX(), _coinSprite.getY(), 10.0f};
        if (Physics2D::checkCircleCollision(heroCircle, coinCircle)) {
            _score += 100;
            if (_score > _highScore) _highScore = _score;
            _sound.playCoin();
            _camera.triggerShake(4.0f, 0.3f);
            _particleEngine.emitExplosion(_coinSprite.getX(), _coinSprite.getY(), 16, 0xFFE0, 0x07E0);
            _coinSprite.setPosition(random(40, 260), random(40, 130));
        }
    }

    void render(GFXContext& gfx) override {
        // Process Touch & Gamepad Controls
        _gamepad.update(gfx);
        const auto& btn = _gamepad.getState();

        if (btn.left)  _heroSprite.move(-2.5f, 0);
        if (btn.right) _heroSprite.move(2.5f, 0);
        if (btn.up)    _heroSprite.move(0, -2.5f);
        if (btn.down)  _heroSprite.move(0, 2.5f);

        if (btn.btnA) {
            _particleEngine.emitExplosion(_heroSprite.getX(), _heroSprite.getY(), 12, 0xFFE0, 0xF800);
            _sound.playLaser();
            _camera.triggerShake(8.0f, 0.4f);
        }

        _isTouchActive = gfx.getTouch(&_activeTouchX, &_activeTouchY);

        // Render ALL Layers off-screen into 320x160 16-bit RGB565 sprite buffer
        _layerManager.renderAll(gfx, gfx.getSprite());

        // Render On-Screen Touch Gamepad Controls & Toast Banner into sprite buffer
        _gamepad.render(gfx);
        _uiManager.renderToast(gfx);

        // Single Atomic Hardware SPI DMA Flush (Guaranteed Non-Null Allocation!)
        gfx.pushBuffer();
    }
};

#endif // ANIMATION_APP_H
