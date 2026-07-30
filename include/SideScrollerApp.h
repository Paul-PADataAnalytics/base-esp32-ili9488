#ifndef SIDESCROLLER_APP_H
#define SIDESCROLLER_APP_H

#include "BaseApp.h"
#include "GFXContext.h"
#include "TileSet.h"
#include "TileMap.h"
#include "LayerManager.h"
#include "RetroSprite.h"
#include "SideScrollerAssets.h"
#include <Arduino.h>

/**
 * SideScrollerApp - Realistic Platformer Benchmark Application
 */
class SideScrollerApp : public BaseApp {

private:
    TileSet        _dirtTileSet;
    TileMap        _dirtMap;

    TileSet        _hillsTileSet;
    TileMap        _hillsMap;

    LayerManager   _layerManager;
    AnimatedSprite _armyHero;
    RetroSprite    _pickupStar;

    float _bgScrollX;
    float _mgScrollX;
    float _pickupX;
    float _pickupY;

    uint32_t _scoreCounter;
    uint32_t _frameCount;
    int      _animTick;

public:
    SideScrollerApp() 
        : _dirtTileSet(DIRT_TILESET_32x32, 32, 32, 32, 128),
          _dirtMap(&_dirtTileSet, PLAY_SURFACE_MAP, 16, 10),
          _hillsTileSet(DIRT_TILESET_32x32, 32, 32, 32, 128),
          _hillsMap(&_hillsTileSet, HILLS_BACKGROUND_MAP, 16, 10),
          _armyHero(ARMY_RUNNER_8FRAMES, 16, 16, 8, 30.0f, C_TRANS),
          _pickupStar(PICKUP_STAR_16x16, 16, 16, C_TRANS),
          _bgScrollX(0.0f), _mgScrollX(0.0f),
          _pickupX(480.0f), _pickupY(130.0f),
          _scoreCounter(0), _frameCount(0), _animTick(0) {}

    void setup(GFXContext& gfx) override {
        // Player in middle of screen (X=240, Y=176 on top of play surface at Y=192)
        _armyHero.setPosition(240, 176);
        _armyHero.setScale(2.0f, 2.0f);
        _armyHero.setPivot(8, 8);
        _armyHero.pause(); // Manual step every 2 game frames

        _pickupStar.setPosition(_pickupX, _pickupY);
        _pickupStar.setScale(1.5f, 1.5f);
        _pickupStar.setPivot(8, 8);

        // --- Layer Pipeline Setup ---

        // 1. BACKGROUND LAYER (Rolling Hills moving at 4 px/sec)
        _layerManager.addLayer("BackgroundHills", LayerRole::BACKGROUND, 0, [this](GFXContext& gfx, LGFX_Sprite* buffer, Layer& layer) {
            gfx.fillScreen(gfx.color565(135, 206, 235)); // Sky Blue Background
            int offset = (int)_bgScrollX % 32;
            // Hills use transparent key 0x0000 (C_TRANS) to let sky blue show through
            _hillsMap.render(gfx.getSprite(), offset, 0, gfx.getBandY(), C_TRANS);
            _hillsMap.render(gfx.getSprite(), offset + 480, 0, gfx.getBandY(), C_TRANS);
        });

        // 2. MIDGROUND LAYER (Play Surface Dirt Moving at 8 px/sec)
        _layerManager.addLayer("PlaySurface", LayerRole::WORLD_MAP, 0, [this](GFXContext& gfx, LGFX_Sprite* buffer, Layer& layer) {
            int offset = (int)_mgScrollX % 32;
            // Dirt surface uses solid opaque blit (0xFFFF)
            _dirtMap.render(gfx.getSprite(), offset, 0, gfx.getBandY(), 0xFFFF);
            _dirtMap.render(gfx.getSprite(), offset + 480, 0, gfx.getBandY(), 0xFFFF);
        });

        // 3. ENTITIES LAYER (Army Hero & Floating Pickup Star)
        _layerManager.addLayer("Entities", LayerRole::ENTITIES, 0, [this](GFXContext& gfx, LGFX_Sprite* buffer, Layer& layer) {
            _armyHero.render(gfx.getSprite(), gfx.getBandY());
            _pickupStar.render(gfx.getSprite(), gfx.getBandY());
        });

        // 4. FOREGROUND UI LAYER (Stationary Top Corner 5-Digit Score Counter)
        _layerManager.addLayer("ForegroundUI", LayerRole::UI_OVERLAY, 0, [this](GFXContext& gfx, LGFX_Sprite* buffer, Layer& layer) {
            // Top HUD Bar Box
            gfx.fillRectDirect(10, 10, 160, 32, gfx.color565(20, 30, 50));
            gfx.drawRectDirect(10, 10, 160, 32, 0xFFFF);

            char scoreBuf[16];
            snprintf(scoreBuf, sizeof(scoreBuf), "SCORE:%05lu", _scoreCounter % 100000);
            gfx.drawTextDirect(scoreBuf, 20, 18, gfx.color565(255, 255, 0), 2, gfx.color565(20, 30, 50));
        });
    }

    void update(float deltaTime) override {
        _frameCount++;
        _scoreCounter++; // Increments by 1 every frame!

        // 1. Parallax Scroll Math:
        // Background moves right-to-left at 4 pixels per second
        _bgScrollX -= 4.0f * deltaTime;
        if (_bgScrollX <= -480.0f) _bgScrollX += 480.0f;

        // Play Surface moves right-to-left at 8 pixels per second
        _mgScrollX -= 8.0f * deltaTime;
        if (_mgScrollX <= -480.0f) _mgScrollX += 480.0f;

        // 2. Animated Hero: Change frame every 2 game frames
        if (_frameCount % 2 == 0) {
            int nextFrame = (_armyHero.getCurrentFrame() + 1) % 8;
            _armyHero.setFrame(nextFrame);
        }

        // 3. Floating Pickup Item moving at 8 px/sec (matching play surface)
        _pickupX -= 8.0f * deltaTime;
        if (_pickupX < -32.0f) {
            _pickupX = 520.0f; // Wrap around to right side
        }
        _pickupStar.setPosition(_pickupX, _pickupY);
    }

    void render(GFXContext& gfx) override {
        // Render 4-band 0% flicker double-buffer loop
        for (int band = 0; band < 4; band++) {
            int bandY = band * 80;
            gfx.setBandY(bandY);

            _layerManager.renderAll(gfx, gfx.getSprite());

            gfx.pushBuffer();
        }
    }
};

#endif // SIDESCROLLER_APP_H
