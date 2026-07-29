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
#include <Arduino.h>

RTC_DATA_ATTR static int rtcWakeupCount = 0;

/**
 * Retro Game Engine v1.3 - Complete Multi-Layer & Affine Matrix Engine Demo
 */
class AnimationApp : public BaseApp {

private:
    SleepManager   _sleepManager;
    TileSet        _tileSet;
    TileMap        _tileMap;
    LayerManager   _layerManager;
    ParticleEngine _particleEngine;
    VirtualGamepad _gamepad;

    RetroSprite    _heroSprite;
    AnimatedSprite _coinSprite;

    float _bgRotation;
    float _bgScrollX;
    float _heroScale;
    float _scaleDirection;

public:
    AnimationApp() 
        : _sleepManager(20, SleepMode::DEEP_SLEEP),
          _tileSet(RETRO_TILESET_32x32, 16, 16, 32, 32),
          _tileMap(&_tileSet, RETRO_LEVEL_MAP, 16, 10),
          _particleEngine(150),
          _heroSprite(ICON_POWER_16x16, 16, 16, 0x0000),
          _coinSprite(RETRO_TILESET_32x32, 16, 16, 4, 8.0f, 0x0000),
          _bgRotation(0.0f), _bgScrollX(0.0f),
          _heroScale(2.0f), _scaleDirection(1.0f) {}

    void setup(GFXContext& gfx) override {
        _sleepManager.onPreSleep([this]() {
            Serial.println("[RETRO ENGINE] Saving game state before sleep...");
        });

        _sleepManager.onPostWake([this]() {
            rtcWakeupCount++;
            Serial.printf("[RETRO ENGINE] Restored state after wake! Wakeups: %d\n", rtcWakeupCount);
        });

        _sleepManager.checkAndNotifyDeepSleepWakeup();

        // Configure Hero Sprite Initial State
        _heroSprite.setPosition(180, 120);
        _heroSprite.setScale(2.0f, 2.0f);
        _heroSprite.setPivot(8, 8);

        // Configure Animated Coin Sprite Initial State
        _coinSprite.setPosition(280, 100);
        _coinSprite.setScale(1.8f, 1.8f);
        _coinSprite.setPivot(8, 8);

        // --- Multi-Layer Pipeline Configuration ---

        // 1. BACKGROUND LAYER (Renders at lowest depth below everything)
        _layerManager.addLayer("Background", LayerRole::BACKGROUND, 0, [this](GFXContext& gfx, LGFX_Sprite* buffer, Layer& layer) {
            buffer->fillScreen(gfx.color565(6, 14, 25));
            // Render rotating starfield background using layer's rotation angle
            for (int i = 0; i < 8; i++) {
                float ang = (layer.getRotation() + i * 45) * DEG_TO_RAD;
                int x2 = 240 + cos(ang) * 120;
                int y2 = 160 + sin(ang) * 120;
                buffer->drawLine(240, 160, x2, y2, gfx.color565(20, 40, 70));
            }
        });

        // 2. WORLD MAP LAYER (TileMap grid level geometry)
        _layerManager.addLayer("TileMap", LayerRole::WORLD_MAP, 0, [this](GFXContext& gfx, LGFX_Sprite* buffer, Layer& layer) {
            _tileMap.render(buffer, (int)layer.getTranslationX(), 0, 0x0000);
        });

        // 3. ENTITIES LAYER (Dynamic Sprites, Hero, Coins, Particles)
        _layerManager.addLayer("Entities", LayerRole::ENTITIES, 0, [this](GFXContext& gfx, LGFX_Sprite* buffer, Layer& layer) {
            // Render Hero Sprite with affine transform
            _heroSprite.render(buffer);

            // Render Animated Coin Sprite
            _coinSprite.render(buffer);

            // Render Particles
            _particleEngine.render(buffer);
        });

        // 4. FOREGROUND LAYER (Renders ABOVE world map & entities for tree canopy occlusion!)
        _layerManager.addLayer("Foreground", LayerRole::FOREGROUND, 0, [this](GFXContext& gfx, LGFX_Sprite* buffer, Layer& layer) {
            // Render tree tops (Tile 3) over entities
            _tileSet.drawTile(buffer, 3, 80 + (int)layer.getTranslationX(), 20, 0x0000);
            _tileSet.drawTile(buffer, 3, 176 + (int)layer.getTranslationX(), 20, 0x0000);
        });

        // 5. UI OVERLAY LAYER (Renders on top of everything: HUD, On-Screen D-Pad, Score)
        _layerManager.addLayer("UIOverlay", LayerRole::UI_OVERLAY, 0, [this](GFXContext& gfx, LGFX_Sprite* buffer, Layer& layer) {
            buffer->fillRect(0, 0, 480, 28, gfx.color565(12, 24, 45));
            buffer->drawRect(0, 0, 480, 28, 0x07FF);
        });

        _sleepManager.resetInactivityTimer();
    }

    void update(float deltaTime) override {
        // 1. Independent Background Layer Rotation & World Map Scrolling
        Layer* bgLayer   = _layerManager.getLayer("Background");
        Layer* mapLayer  = _layerManager.getLayer("TileMap");
        Layer* fgLayer   = _layerManager.getLayer("Foreground");

        if (bgLayer) bgLayer->rotate(15.0f * deltaTime);
        
        _bgScrollX -= 8.0f * deltaTime;
        if (_bgScrollX <= -32.0f) _bgScrollX += 32.0f;

        if (mapLayer) mapLayer->setTranslation(_bgScrollX, 0);
        if (fgLayer)  fgLayer->setTranslation(_bgScrollX * 1.5f, 0); // Parallax foreground!

        // 2. Hero Sprite Pulsing Scale, Rotation, & Skewing
        _heroSprite.rotate(90.0f * deltaTime);
        _heroScale += 0.5f * _scaleDirection * deltaTime;
        if (_heroScale >= 2.5f || _heroScale <= 1.2f) {
            _scaleDirection *= -1.0f;
        }
        _heroSprite.setScale(_heroScale, _heroScale);
        _heroSprite.setSkew(sin(_bgRotation * DEG_TO_RAD) * 0.2f, 0.0f); // Skewing effect!

        // 3. Update Animated Coin Sprite
        _coinSprite.update(deltaTime);

        // 4. Update Gamepad & Particles
        _particleEngine.update(deltaTime);
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
            _sleepManager.resetInactivityTimer();
        }

        int tx, ty;
        if (gfx.getTouch(&tx, &ty)) {
            _sleepManager.resetInactivityTimer();
        }

        // Check Auto-Sleep Trigger
        if (_sleepManager.shouldAutoSleep()) {
            _sleepManager.triggerSleep(gfx);
            return;
        }

        // Composite All Layers in Depth Order onto Sprite Buffer
        _layerManager.renderAll(gfx, gfx.getSprite());
        gfx.pushBuffer();

        // Render On-Screen Touch Gamepad Controls
        _gamepad.render(gfx);

        // Render UI Text Overlay
        gfx.drawTextDirect("Retro Engine v1.3", 15, 6, gfx.color565(0, 255, 255), 2);

        uint32_t secRemaining = _sleepManager.getInactivitySecondsRemaining();
        char sleepBuf[32];
        snprintf(sleepBuf, sizeof(sleepBuf), "DeepSleep: %lds ", (long)secRemaining);
        gfx.drawTextDirect(sleepBuf, 290, 6, gfx.color565(255, 165, 0), 2);

        if (rtcWakeupCount > 0) {
            char wakeBuf[32];
            snprintf(wakeBuf, sizeof(wakeBuf), "Wakes: %d", rtcWakeupCount);
            gfx.drawTextDirect(wakeBuf, 15, 295, gfx.color565(0, 255, 120), 2);
        }
    }
};

#endif // ANIMATION_APP_H
