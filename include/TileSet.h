#ifndef TILESET_H
#define TILESET_H

#include <Arduino.h>
#include <LovyanGFX.hpp>

/**
 * TileSet - High-Performance Retro Graphics Atlas Module
 * 
 * Corrects LovyanGFX SPI byte-ordering to restore true 16-bit RGB565 tile colors.
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

        // 3. Fast Row Transfer into Sprite Buffer with Byte-Swap Correction
        uint16_t rowBuffer[64];
        int copyW = (_tileWidth > 64) ? 64 : _tileWidth;

        for (int y = startY; y < startY + drawH; y++) {
            int flashRowOffset = (srcY + y) * atlasStride + srcX;
            int destY = drawY + (y - startY);

            for (int x = 0; x < copyW; x++) {
                uint16_t rawPixel = pgm_read_word(&_bitmap[flashRowOffset + x]);
                // Byte-swap for LovyanGFX 16-bit RGB565 sprite buffer
                rowBuffer[x] = (rawPixel >> 8) | (rawPixel << 8);
            }

            if (transparentKey == 0xFFFF) {
                // Solid tile blit (Opaque grass & dirt tiles)
                canvas->pushImage(posX, destY, copyW, 1, rowBuffer);
            } else {
                // Transparent tile blit (Rolling hills & pine trees)
                canvas->pushImage(posX, destY, copyW, 1, rowBuffer, transparentKey);
            }
        }
    }
};

#endif // TILESET_H
