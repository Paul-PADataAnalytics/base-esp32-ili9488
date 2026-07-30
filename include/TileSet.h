#ifndef TILESET_H
#define TILESET_H

#include <Arduino.h>
#include <LovyanGFX.hpp>

/**
 * TileSet - High-Performance Direct-Memory Pixel Blitter Module
 * 
 * Writes byte-swapped 16-bit RGB565 pixels directly into sprite RAM for ILI9488 SPI DMA.
 */
class TileSet {
private:
    const uint16_t* _bitmap;
    int _tileWidth;
    int _tileHeight;
    int _columns;
    int _rows;

public:
    TileSet(const uint16_t* bitmap, int tileW, int tileH, int atlasW, int atlasH)
        : _bitmap(bitmap), _tileWidth(tileW), _tileHeight(tileH),
          _columns(atlasW / tileW), _rows(atlasH / tileH) {}

    int getTileWidth() const { return _tileWidth; }
    int getTileHeight() const { return _tileHeight; }
    int getColumns() const { return _columns; }
    int getRows() const { return _rows; }

    void drawTile(LovyanGFX* canvas, int tileIndex, int posX, int posY, int bandY = 0, uint16_t transparentKey = 0xFFFF) const {
        if (tileIndex < 0 || !_bitmap || !canvas) return;

        int localY = posY - bandY;
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

        // 3. Direct RAM Pointer Writing to 16-Bit RGB565 Sprite Buffer
        LGFX_Sprite* sprite = (LGFX_Sprite*)canvas;
        uint16_t* buf = (uint16_t*)sprite->getBuffer();
        if (!buf) return;

        int canvasW = canvas->width();

        for (int y = startY; y < startY + drawH; y++) {
            int flashRowOffset = (srcY + y) * atlasStride + srcX;
            int destY = drawY + (y - startY);
            int destRowOffset = destY * canvasW;

            for (int x = 0; x < _tileWidth; x++) {
                int destX = posX + x;
                if (destX >= 0 && destX < canvasW) {
                    uint16_t pixel = pgm_read_word(&_bitmap[flashRowOffset + x]);
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
