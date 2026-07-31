#ifndef TILESET_H
#define TILESET_H

#if defined(PLATFORM_LINUX) || !defined(ARDUINO)
#include "Arduino_Linux.h"
#else
#include <Arduino.h>
#endif
#include <LovyanGFX.hpp>

/**
 * TileSet - High-Performance Direct-Memory Pixel Blitter Module
 * 
 * Optimization: Tile atlas is copied from SPI Flash (PROGMEM) into internal
 * ESP32 DRAM on construction. All subsequent pixel reads are direct 240 MHz
 * DRAM pointer reads instead of slow pgm_read_word() SPI Flash reads.
 * This eliminates the ~15x-20x read latency penalty on every tile pixel.
 */
class TileSet {
private:
    const uint16_t* _flashBitmap;   // Original PROGMEM pointer (kept for reference)
    uint16_t*       _ramBitmap;     // DRAM-cached copy of the atlas
    int _tileWidth;
    int _tileHeight;
    int _columns;
    int _rows;
    int _atlasPixels;

public:
    TileSet(const uint16_t* bitmap, int tileW, int tileH, int atlasW, int atlasH)
        : _flashBitmap(bitmap), _ramBitmap(nullptr),
          _tileWidth(tileW), _tileHeight(tileH),
          _columns(atlasW / tileW), _rows(atlasH / tileH),
          _atlasPixels(atlasW * atlasH)
    {
        // Allocate DRAM buffer and copy atlas from SPI Flash into DRAM
        _ramBitmap = (uint16_t*)malloc(_atlasPixels * sizeof(uint16_t));
        if (_ramBitmap) {
            for (int i = 0; i < _atlasPixels; i++) {
                _ramBitmap[i] = pgm_read_word(&bitmap[i]);
            }
            Serial.printf("[TILESET] Atlas cached: %d bytes in DRAM\n", _atlasPixels * 2);
        } else {
            Serial.printf("[TILESET] ERROR: Failed to allocate %d bytes for atlas DRAM cache!\n",
                          _atlasPixels * 2);
        }
    }

    ~TileSet() {
        if (_ramBitmap) {
            free(_ramBitmap);
            _ramBitmap = nullptr;
        }
    }

    int getTileWidth()  const { return _tileWidth; }
    int getTileHeight() const { return _tileHeight; }
    int getColumns()    const { return _columns; }
    int getRows()       const { return _rows; }

    void drawTile(LovyanGFX* canvas, int tileIndex, int posX, int posY, int bandY = 0, uint16_t transparentKey = 0xFFFF) const {
        if (tileIndex < 0 || !_ramBitmap || !canvas) return;

        int localY  = posY - bandY;
        int canvasH = canvas->height();

        // 1. Full Vertical Culling Check
        if (localY + _tileHeight <= 0 || localY >= canvasH) return;

        int srcX = (tileIndex % _columns) * _tileWidth;
        int srcY = (tileIndex / _columns) * _tileHeight;
        int atlasStride = _columns * _tileWidth;

        // 2. High-Speed Band Clipping Logic
        int startY = 0;
        int drawY  = localY;
        int drawH  = _tileHeight;

        if (localY < 0) {
            startY = -localY;
            drawY  = 0;
            drawH  -= startY;
        }

        if (drawY + drawH > canvasH) {
            drawH = canvasH - drawY;
        }

        if (drawH <= 0) return;

        // 3. Direct DRAM Pointer Read into Sprite Buffer (no pgm_read_word overhead)
        LGFX_Sprite* sprite = (LGFX_Sprite*)canvas;
        uint16_t* buf = (uint16_t*)sprite->getBuffer();
        if (!buf) return;

        int canvasW = canvas->width();

        for (int y = startY; y < startY + drawH; y++) {
            int ramRowOffset  = (srcY + y) * atlasStride + srcX;
            int destY         = drawY + (y - startY);
            int destRowOffset = destY * canvasW;

            for (int x = 0; x < _tileWidth; x++) {
                int destX = posX + x;
                if (destX >= 0 && destX < canvasW) {
                    // Direct DRAM read — ~15-20x faster than pgm_read_word()
                    uint16_t pixel = _ramBitmap[ramRowOffset + x];
                    if (pixel != transparentKey) {
                        // Byte swap for ILI9488 SPI DMA transmission
                        buf[destRowOffset + destX] = (pixel >> 8) | (pixel << 8);
                    }
                }
            }
        }
    }
};

#endif // TILESET_H
