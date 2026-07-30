#ifndef SIDESCROLLER_APP_H
#define SIDESCROLLER_APP_H

#include <Arduino.h>
#include "GFXContext.h"
#include "LayerManager.h"
#include "TileMap.h"
#include "SideScrollerAssets.h"

/**
 * SideScrollerApp - Realistic Side-Scroller Platformer Benchmark Application
 *
 * ## Responsibility
 * This class is responsible ONLY for game logic and content:
 *   - Declaring layers with their Y-ranges and render callbacks.
 *   - Calling layer->markDirty() when a layer's content changes.
 *   - Calling gfx.renderBands() once per frame.
 *
 * ## What this class does NOT contain
 * No SPI bus management, no band loop, no dirty-flag arithmetic, no DMA
 * orchestration. All of that lives in GFXContext::renderBands() and
 * LayerManager::computeBandDirtyFlags() and is fully reusable.
 *
 * ## Scroll Rates (Benchmark-Frozen)
 *   - Foreground / play surface: 1 pixel per 2 frames
 *   - Background / parallax hills: 1 pixel per 8 frames
 *
 * ## Y-Layout (Benchmark-Frozen, 480x320 canvas, 80px bands)
 *   Band 0  Y=0..79    Sky + parallax hills + HUD score
 *   Band 1  Y=80..159  Sky + hero runner (top half)
 *   Band 2  Y=160..239 Ground tiles + hero runner (bottom half) + star
 *   Band 3  Y=240..319 Ground tiles (deep dirt)
 */
class SideScrollerApp {
private:
    LayerManager _layerManager;

    TileSet _dirtTileSet;
    TileMap _dirtMap;
    TileSet _hillsTileSet;
    TileMap _hillsMap;

    int _bgScrollX;
    int _mgScrollX;
    int _fgScrollX;
    int _heroFrame;
    int _animFrameCounter;
    int _scrollFrameCounter;
    int _score;

    // Cached layer pointers set in setup() for fast markDirty() calls in update()
    Layer* _layerBg;
    Layer* _layerGround;
    Layer* _layerEntities;
    Layer* _layerHUD;

public:
    SideScrollerApp()
        : _dirtTileSet(DIRT_TILESET_32x32, 32, 32, 32, 192),
          _dirtMap(&_dirtTileSet, PLAY_SURFACE_MAP, 16, 10),
          _hillsTileSet(DIRT_TILESET_32x32, 32, 32, 32, 192),
          _hillsMap(&_hillsTileSet, HILLS_BACKGROUND_MAP, 16, 10),
          _bgScrollX(0), _mgScrollX(0), _fgScrollX(0),
          _heroFrame(0), _animFrameCounter(0), _scrollFrameCounter(0), _score(0),
          _layerBg(nullptr), _layerGround(nullptr),
          _layerEntities(nullptr), _layerHUD(nullptr)
    {}

    void setup(GFXContext& gfx) {
        _layerManager.clearLayers();

        // ------------------------------------------------------------------
        // Layer 0: Sky & Parallax Rolling Hills
        //   Y-range: 0..192 (sky fills top 6 bands of map; hill base at Y=192)
        //   Dirty when: bgScrollX changes (every 8 frames)
        // ------------------------------------------------------------------
        _layerManager.addLayer("BackgroundHills", LayerRole::BACKGROUND, 0,
            [this](GFXContext& gfx, LGFX_Sprite* buf, Layer& layer) {
                int bandY = gfx.getBandY();
                if (bandY >= 192) return;

                buf->fillScreen(gfx.color565(135, 206, 235));
                int offset = _bgScrollX % 512;
                if (offset > 0) offset -= 512;
                _hillsMap.render(buf, offset,       0, bandY, C_TRANS);
                _hillsMap.render(buf, offset + 512, 0, bandY, C_TRANS);
            },
            /*minDrawY=*/ 0, /*maxDrawY=*/ 192);
        _layerBg = _layerManager.getLayer("BackgroundHills");

        // ------------------------------------------------------------------
        // Layer 1: Midground Play Surface (Dirt & Grass Tiles)
        //   Y-range: 160..320 (grass row starts at map row 6, Y=192; dirt fills below)
        //   Dirty when: mgScrollX changes (every 2 frames)
        // ------------------------------------------------------------------
        _layerManager.addLayer("PlaySurface", LayerRole::WORLD_MAP, 0,
            [this](GFXContext& gfx, LGFX_Sprite* buf, Layer& layer) {
                int bandY = gfx.getBandY();
                if (bandY < 160) return;

                int offset = _mgScrollX % 512;
                if (offset > 0) offset -= 512;
                _dirtMap.render(buf, offset,       0, bandY, 0xFFFF);
                _dirtMap.render(buf, offset + 512, 0, bandY, 0xFFFF);
            },
            /*minDrawY=*/ 160, /*maxDrawY=*/ 320);
        _layerGround = _layerManager.getLayer("PlaySurface");

        // ------------------------------------------------------------------
        // Layer 2: Entities — Animated Hero Runner & Floating Collectible Star
        //   Y-range: 80..240 (hero is 32px tall scaled 2x at Y=176..208; star at Y=145..161)
        //   Dirty when: heroFrame changes (every 2 frames) or mgScrollX changes
        // ------------------------------------------------------------------
        _layerManager.addLayer("Entities", LayerRole::ENTITIES, 0,
            [this](GFXContext& gfx, LGFX_Sprite* buf, Layer& layer) {
                int bandY = gfx.getBandY();
                if (bandY < 80 || bandY >= 240) return;

                // A. Floating Gold Star Pickup
                int starX = 360 + (_fgScrollX % 512);
                if (starX < -16) starX += 512;
                for (int sy = 0; sy < 16; sy++) {
                    int destY = 145 + sy - bandY;
                    if (destY < 0 || destY >= buf->height()) continue;
                    for (int sx = 0; sx < 16; sx++) {
                        uint16_t p = pgm_read_word(&PICKUP_STAR_16x16[sy * 16 + sx]);
                        if (p == C_TRANS) continue;
                        int destX = starX + sx;
                        if (destX < 0 || destX >= buf->width()) continue;
                        uint16_t* b = (uint16_t*)buf->getBuffer();
                        b[destY * buf->width() + destX] = (p >> 8) | (p << 8);
                    }
                }

                // B. Hero Runner (2x scale, centred at X=240, ground contact Y=192)
                const uint16_t* framePtr = &ARMY_RUNNER_8FRAMES[_heroFrame * 16 * 16];
                for (int hy = 0; hy < 16; hy++) {
                    for (int sy = 0; sy < 2; sy++) {
                        int destY = 176 + hy * 2 + sy - bandY;
                        if (destY < 0 || destY >= buf->height()) continue;
                        for (int hx = 0; hx < 16; hx++) {
                            uint16_t p = pgm_read_word(&framePtr[hy * 16 + hx]);
                            if (p == C_TRANS) continue;
                            for (int sx = 0; sx < 2; sx++) {
                                int destX = 240 + hx * 2 + sx - 16;
                                if (destX < 0 || destX >= buf->width()) continue;
                                uint16_t* b = (uint16_t*)buf->getBuffer();
                                b[destY * buf->width() + destX] = (p >> 8) | (p << 8);
                            }
                        }
                    }
                }
            },
            /*minDrawY=*/ 80, /*maxDrawY=*/ 240);
        _layerEntities = _layerManager.getLayer("Entities");

        // ------------------------------------------------------------------
        // Layer 3: Foreground HUD — Score Box
        //   Y-range: 0..50 (score box sits in top-left, 42px tall including border)
        //   Dirty when: score changes (every frame)
        // ------------------------------------------------------------------
        _layerManager.addLayer("ForegroundHUD", LayerRole::UI_OVERLAY, 0,
            [this](GFXContext& gfx, LGFX_Sprite* buf, Layer& layer) {
                if (gfx.getBandY() != 0) return;
                buf->fillRect(10, 10, 150, 32, gfx.color565(20, 30, 50));
                buf->drawRect(10, 10, 150, 32, gfx.color565(255, 215, 0));
                char scoreStr[16];
                snprintf(scoreStr, sizeof(scoreStr), "SCORE:%05d", _score);
                buf->setTextColor(gfx.color565(255, 255, 255));
                buf->setTextSize(2);
                buf->setCursor(18, 18);
                buf->print(scoreStr);
            },
            /*minDrawY=*/ 0, /*maxDrawY=*/ 50);
        _layerHUD = _layerManager.getLayer("ForegroundHUD");
    }

    // ------------------------------------------------------------------------
    // update() — Game logic only. No rendering. No hardware calls.
    //
    // The ONLY job regarding the render system is calling markDirty() on layers
    // whose content has changed this frame. The engine works out which bands
    // are affected automatically.
    // ------------------------------------------------------------------------
    void update(float deltaTime) {
        _scrollFrameCounter++;

        // Play surface scrolls 1px per 2 frames
        if (_scrollFrameCounter % 2 == 0) {
            _mgScrollX--;
            _fgScrollX--;
            if (_layerGround)   _layerGround->markDirty();
            if (_layerEntities) _layerEntities->markDirty();
        }

        // Background scrolls 1px per 8 frames
        if (_scrollFrameCounter % 8 == 0) {
            _bgScrollX--;
            if (_layerBg) _layerBg->markDirty();
        }

        // Hero animation advances every 2 frames
        _animFrameCounter++;
        if (_animFrameCounter >= 2) {
            _animFrameCounter = 0;
            _heroFrame = (_heroFrame + 1) % 8;
            if (_layerEntities) _layerEntities->markDirty();
        }

        // Score increments every frame
        _score = (_score + 1) % 100000;
        if (_layerHUD) _layerHUD->markDirty();
    }

    // ------------------------------------------------------------------------
    // render() — Fully general. Zero scene-specific logic.
    //
    // 1. Ask LayerManager which bands are dirty (based on layer Y-ranges +
    //    stateVersion counters — no hardcoded band/variable mapping).
    // 2. Hand control to GFXContext::renderBands() which runs the DMA
    //    double-buffer pipeline for all dirty bands.
    // ------------------------------------------------------------------------
    void render(GFXContext& gfx) {
        bool bandDirty[LayerManager::MAX_BANDS] = {};
        _layerManager.computeBandDirtyFlags(bandDirty, gfx.getNumBands(), gfx.getBandHeight());
        gfx.renderBands(_layerManager, bandDirty);
    }
};

#endif // SIDESCROLLER_APP_H
