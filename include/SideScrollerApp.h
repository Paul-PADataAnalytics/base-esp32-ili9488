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
 * Performance Optimized Version:
 * - 40 MHz Hardware SPI DMA transfers (100% hardware stable)
 * - Band-based layer culling for maximum FPS
 * - 512px Tilemap wrap for 100% butter-smooth seamless scrolling
 */
class SideScrollerApp {
private:
    LayerManager _layerManager;

    TileSet _dirtTileSet;
    TileMap _dirtMap;

    TileSet _hillsTileSet;
    TileMap _hillsMap;

    int _bgScrollX; // Background parallax scroll position in integer pixels
    int _mgScrollX; // Midground play surface scroll position in integer pixels
    int _fgScrollX; // Foreground pickup item scroll position in integer pixels

    int _heroFrame;
    int _animFrameCounter;
    int _scrollFrameCounter;
    int _score;

public:
    SideScrollerApp() 
        : _dirtTileSet(DIRT_TILESET_32x32, 32, 32, 32, 192),
          _dirtMap(&_dirtTileSet, PLAY_SURFACE_MAP, 16, 10),
          _hillsTileSet(DIRT_TILESET_32x32, 32, 32, 32, 192),
          _hillsMap(&_hillsTileSet, HILLS_BACKGROUND_MAP, 16, 10),
          _bgScrollX(0), _mgScrollX(0), _fgScrollX(0),
          _heroFrame(0), _animFrameCounter(0), _scrollFrameCounter(0), _score(0) {}

    void setup(GFXContext& gfx) {
        _layerManager.clearLayers();

        // 1. Layer 0: Sky & Parallax Rolling Hills (1 pixel per 8 frames)
        _layerManager.addLayer("BackgroundHills", LayerRole::BACKGROUND, 0, [this](GFXContext& gfx, LGFX_Sprite* buffer, Layer& layer) {
            int bandY = gfx.getBandY();
            // Cull Background layer if band is strictly below Y=192
            if (bandY >= 192) return;

            // Fill sky blue color
            buffer->fillScreen(gfx.color565(135, 206, 235));

            // Seamless 512px map wrap (16 cols * 32px = 512px map width)
            int offset = _bgScrollX % 512;
            if (offset > 0) offset -= 512;

            _hillsMap.render(buffer, offset, 0, bandY, C_TRANS);
            _hillsMap.render(buffer, offset + 512, 0, bandY, C_TRANS);
        });

        // 2. Layer 1: Midground Play Surface Dirt & Grass Tiles (1 pixel per 2 frames)
        _layerManager.addLayer("PlaySurface", LayerRole::WORLD_MAP, 0, [this](GFXContext& gfx, LGFX_Sprite* buffer, Layer& layer) {
            int bandY = gfx.getBandY();
            // Cull Play Surface layer if band is strictly above Y=160
            if (bandY < 160) return;

            // Seamless 512px map wrap (16 cols * 32px = 512px map width)
            int offset = _mgScrollX % 512;
            if (offset > 0) offset -= 512;

            _dirtMap.render(buffer, offset, 0, bandY, 0xFFFF);
            _dirtMap.render(buffer, offset + 512, 0, bandY, 0xFFFF);
        });

        // 3. Layer 2: Animated Hero Runner & Floating Collectible Pickup Star
        _layerManager.addLayer("Entities", LayerRole::ENTITIES, 0, [this](GFXContext& gfx, LGFX_Sprite* buffer, Layer& layer) {
            int bandY = gfx.getBandY();
            // Cull Entities layer if band is outside Y=80..240
            if (bandY < 80 || bandY >= 240) return;

            // A. Draw Floating Gold Star Pickup Item
            int starX = 360 + (_fgScrollX % 512);
            if (starX < -16) starX += 512;

            // Draw pickup star using row blitting directly on buffer
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

            // B. Draw Hero Runner Aligned Firmly to Top of Dirt Ground Surface (Y = 176)
            const uint16_t* framePtr = &ARMY_RUNNER_8FRAMES[_heroFrame * 16 * 16];
            for (int hy = 0; hy < 16; hy++) {
                for (int scaleY = 0; scaleY < 2; scaleY++) {
                    int destY = 176 + (hy * 2 + scaleY) - bandY;
                    if (destY >= 0 && destY < buffer->height()) {
                        for (int hx = 0; hx < 16; hx++) {
                            uint16_t p = pgm_read_word(&framePtr[hy * 16 + hx]);
                            if (p != C_TRANS) {
                                for (int scaleX = 0; scaleX < 2; scaleX++) {
                                    int destX = 240 + (hx * 2 + scaleX) - 16; // Centered
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

        // 4. Layer 3: Foreground UI Stationary HUD Score Box
        _layerManager.addLayer("ForegroundHUD", LayerRole::UI_OVERLAY, 0, [this](GFXContext& gfx, LGFX_Sprite* buffer, Layer& layer) {
            int bandY = gfx.getBandY();
            // HUD is only in Band 0 (Y = 0..80)
            if (bandY != 0) return;

            // Dark semi-transparent HUD container box
            buffer->fillRect(10, 10, 150, 32, gfx.color565(20, 30, 50));
            buffer->drawRect(10, 10, 150, 32, gfx.color565(255, 215, 0));

            // 5-Digit Score Counter String
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

        // 1. Play Surface / Midground: 1 pixel per 2 frames
        if (_scrollFrameCounter % 2 == 0) {
            _mgScrollX--;
            _fgScrollX--;
        }

        // 2. Background Hills: 1 pixel per 8 frames
        if (_scrollFrameCounter % 8 == 0) {
            _bgScrollX--;
        }

        // 3. Hero Runner 8-frame Animation Timing (Updates frame every 2 game frames)
        _animFrameCounter++;
        if (_animFrameCounter >= 2) {
            _animFrameCounter = 0;
            _heroFrame = (_heroFrame + 1) % 8;
        }

        // 4. Increment 5-Digit Score Counter by 1 each frame
        _score = (_score + 1) % 100000;
    }

    void render(GFXContext& gfx) {
        // Multi-Band Render Loop
        int totalHeight = gfx.getHeight();
        int bandHeight  = 80;

        for (int bandY = 0; bandY < totalHeight; bandY += bandHeight) {
            gfx.setBandY(bandY);
            _layerManager.renderAll(gfx, gfx.getSprite());
            gfx.pushBuffer();
        }
    }
};

#endif // SIDESCROLLER_APP_H
