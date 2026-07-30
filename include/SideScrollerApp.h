#ifndef SIDESCROLLER_APP_H
#define SIDESCROLLER_APP_H

#include <Arduino.h>
#include "GFXContext.h"
#include "LayerManager.h"
#include "TileMap.h"
#include "SideScrollerAssets.h"

/**
 * Realistic Side-Scroller Platformer Benchmark Application
 *
 * Performance Optimizations Active:
 * 1. 40 MHz Hardware SPI DMA transfers (hardware-stable maximum)
 * 2. DRAM Tile Atlas Cache (12 KB) - eliminates pgm_read_word() SPI flash reads
 * 3. Dirty-Rectangle Band Tracking - skips re-render + re-push for unchanged bands
 * 4. DMA Double-Buffering - CPU renders band N+1 while SPI transfers band N
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

    // --- Dirty-Rect Tracking ---
    // Per-band dirty state: true = needs re-render + push this frame
    bool _bandDirty[4];

    // Last rendered value of each tracked variable, per band
    int _lastBgScrollX;    // Tracks when Band 0 & 1 background changes
    int _lastMgScrollX;    // Tracks when Band 2 & 3 foreground changes
    int _lastHeroFrame;    // Tracks when hero sprite changes (Bands 1 & 2)
    int _lastScore;        // Tracks when HUD score changes (Band 0)

public:
    SideScrollerApp()
        : _dirtTileSet(DIRT_TILESET_32x32, 32, 32, 32, 192),
          _dirtMap(&_dirtTileSet, PLAY_SURFACE_MAP, 16, 10),
          _hillsTileSet(DIRT_TILESET_32x32, 32, 32, 32, 192),
          _hillsMap(&_hillsTileSet, HILLS_BACKGROUND_MAP, 16, 10),
          _bgScrollX(0), _mgScrollX(0), _fgScrollX(0),
          _heroFrame(0), _animFrameCounter(0), _scrollFrameCounter(0), _score(0),
          _lastBgScrollX(-1), _lastMgScrollX(-1), _lastHeroFrame(-1), _lastScore(-1)
    {
        for (int i = 0; i < 4; i++) _bandDirty[i] = true; // Force full render on first frame
    }

    void setup(GFXContext& gfx) {
        _layerManager.clearLayers();

        // Layer 0: Sky & Parallax Rolling Hills (background, 1px per 8 frames)
        _layerManager.addLayer("BackgroundHills", LayerRole::BACKGROUND, 0,
            [this](GFXContext& gfx, LGFX_Sprite* buffer, Layer& layer) {
                int bandY = gfx.getBandY();
                if (bandY >= 192) return;

                buffer->fillScreen(gfx.color565(135, 206, 235));

                int offset = _bgScrollX % 512;
                if (offset > 0) offset -= 512;

                _hillsMap.render(buffer, offset,       0, bandY, C_TRANS);
                _hillsMap.render(buffer, offset + 512, 0, bandY, C_TRANS);
            });

        // Layer 1: Midground Play Surface Dirt & Grass Tiles (1px per 2 frames)
        _layerManager.addLayer("PlaySurface", LayerRole::WORLD_MAP, 0,
            [this](GFXContext& gfx, LGFX_Sprite* buffer, Layer& layer) {
                int bandY = gfx.getBandY();
                if (bandY < 160) return;

                int offset = _mgScrollX % 512;
                if (offset > 0) offset -= 512;

                _dirtMap.render(buffer, offset,       0, bandY, 0xFFFF);
                _dirtMap.render(buffer, offset + 512, 0, bandY, 0xFFFF);
            });

        // Layer 2: Animated Hero Runner & Floating Collectible Star
        _layerManager.addLayer("Entities", LayerRole::ENTITIES, 0,
            [this](GFXContext& gfx, LGFX_Sprite* buffer, Layer& layer) {
                int bandY = gfx.getBandY();
                if (bandY < 80 || bandY >= 240) return;

                // A. Floating Gold Star
                int starX = 360 + (_fgScrollX % 512);
                if (starX < -16) starX += 512;

                for (int sy = 0; sy < 16; sy++) {
                    int destY = 145 + sy - bandY;
                    if (destY >= 0 && destY < buffer->height()) {
                        for (int sx = 0; sx < 16; sx++) {
                            uint16_t p = pgm_read_word(&PICKUP_STAR_16x16[sy * 16 + sx]);
                            if (p != C_TRANS) {
                                int destX = starX + sx;
                                if (destX >= 0 && destX < buffer->width()) {
                                    uint16_t* b = (uint16_t*)buffer->getBuffer();
                                    b[destY * buffer->width() + destX] = (p >> 8) | (p << 8);
                                }
                            }
                        }
                    }
                }

                // B. Hero Runner (2x scaled, centered at X=240, ground at Y=192)
                const uint16_t* framePtr = &ARMY_RUNNER_8FRAMES[_heroFrame * 16 * 16];
                for (int hy = 0; hy < 16; hy++) {
                    for (int scaleY = 0; scaleY < 2; scaleY++) {
                        int destY = 176 + (hy * 2 + scaleY) - bandY;
                        if (destY >= 0 && destY < buffer->height()) {
                            for (int hx = 0; hx < 16; hx++) {
                                uint16_t p = pgm_read_word(&framePtr[hy * 16 + hx]);
                                if (p != C_TRANS) {
                                    for (int scaleX = 0; scaleX < 2; scaleX++) {
                                        int destX = 240 + (hx * 2 + scaleX) - 16;
                                        if (destX >= 0 && destX < buffer->width()) {
                                            uint16_t* b = (uint16_t*)buffer->getBuffer();
                                            b[destY * buffer->width() + destX] = (p >> 8) | (p << 8);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            });

        // Layer 3: Foreground HUD Score Box (Band 0 only)
        _layerManager.addLayer("ForegroundHUD", LayerRole::UI_OVERLAY, 0,
            [this](GFXContext& gfx, LGFX_Sprite* buffer, Layer& layer) {
                int bandY = gfx.getBandY();
                if (bandY != 0) return;

                buffer->fillRect(10, 10, 150, 32, gfx.color565(20, 30, 50));
                buffer->drawRect(10, 10, 150, 32, gfx.color565(255, 215, 0));

                char scoreStr[16];
                snprintf(scoreStr, sizeof(scoreStr), "SCORE:%05d", _score);
                buffer->setTextColor(gfx.color565(255, 255, 255));
                buffer->setTextSize(2);
                buffer->setCursor(18, 18);
                buffer->print(scoreStr);
            });
    }

    void update(float deltaTime) {
        _scrollFrameCounter++;

        // Play Surface / Midground: 1 pixel per 2 frames
        if (_scrollFrameCounter % 2 == 0) {
            _mgScrollX--;
            _fgScrollX--;
        }

        // Background Hills: 1 pixel per 8 frames
        if (_scrollFrameCounter % 8 == 0) {
            _bgScrollX--;
        }

        // Hero 8-frame animation: advance every 2 frames
        _animFrameCounter++;
        if (_animFrameCounter >= 2) {
            _animFrameCounter = 0;
            _heroFrame = (_heroFrame + 1) % 8;
        }

        // Increment score
        _score = (_score + 1) % 100000;

        // --- Update Dirty Flags ---
        // Band 0 (Y=0..79):   Sky bg + HUD score → dirty if score or bgScroll changed
        // Band 1 (Y=80..159): Sky bg + hero top  → dirty if bgScroll or heroFrame changed
        // Band 2 (Y=160..239):Ground + hero feet + star → dirty if mgScroll or heroFrame changed
        // Band 3 (Y=240..319):Ground only         → dirty if mgScroll changed

        bool bgChanged   = (_bgScrollX != _lastBgScrollX);
        bool mgChanged   = (_mgScrollX != _lastMgScrollX);
        bool heroChanged = (_heroFrame  != _lastHeroFrame);
        bool scoreChanged= (_score      != _lastScore);

        _bandDirty[0] = bgChanged || scoreChanged;
        _bandDirty[1] = bgChanged || heroChanged;
        _bandDirty[2] = mgChanged || heroChanged;
        _bandDirty[3] = mgChanged;

        // Record last-rendered values
        _lastBgScrollX = _bgScrollX;
        _lastMgScrollX = _mgScrollX;
        _lastHeroFrame = _heroFrame;
        _lastScore     = _score;
    }

    void render(GFXContext& gfx) {
        /**
         * DMA Double-Buffering + Dirty-Rect Render Loop
         *
         * Algorithm:
         *   Frame N has 4 bands (0..3). For each band:
         *     - If dirty: render into the active sprite buffer
         *     - If the PREVIOUS band was dirty (DMA in flight), wait for DMA, then
         *       swap buffers and initiate DMA for the just-rendered band.
         *     - If not dirty: skip both render and push entirely.
         *
         * The key overlap: while DMA transfers band[i], CPU renders band[i+1].
         */
        const int totalHeight = gfx.getHeight();   // 320
        const int bandHeight  = 80;
        const int numBands    = totalHeight / bandHeight; // 4

        bool prevDirty   = false;  // Was the previous band dirty (i.e. is DMA in flight)?
        int  prevBandY   = 0;      // bandY of the in-flight DMA transfer

        for (int band = 0; band < numBands; band++) {
            int bandY = band * bandHeight;

            if (_bandDirty[band]) {
                // --- Render phase: draw into active buffer ---
                // (DMA for previous band may be running concurrently in hardware)
                gfx.setBandY(bandY);
                _layerManager.renderAll(gfx, gfx.getSprite());

                // --- Wait for previous DMA to finish before touching the bus ---
                if (prevDirty) {
                    gfx.waitDMA();
                }

                // --- Push this band via DMA (non-blocking) ---
                gfx.pushBufferDMA();

                // --- Swap to the other buffer so next render doesn't clobber in-flight DMA ---
                gfx.swapBuffer();

                prevDirty = true;
                prevBandY = bandY;
            } else {
                // This band is clean — if DMA is still running let it finish,
                // but don't re-render or re-push this band at all.
                if (prevDirty) {
                    gfx.waitDMA();
                    prevDirty = false;
                }
            }
        }

        // Final flush: wait for the last in-flight DMA if any
        if (prevDirty) {
            gfx.waitDMA();
        }
    }
};

#endif // SIDESCROLLER_APP_H
