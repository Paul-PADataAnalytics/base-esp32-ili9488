#ifndef TILESET_H
#define TILESET_H

#include <Arduino.h>
#include <LovyanGFX.hpp>

/**
 * TileSet - High-Performance Retro Graphics Atlas Module
 * 
 * Uses LovyanGFX fast-path block pushImage transfers instead of pixel loops.
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
        if (localY + _tileHeight < 0 || localY >= canvas->height()) return;

        int srcX = (tileIndex % _columns) * _tileWidth;
        int srcY = (tileIndex / _columns) * _tileHeight;
        int atlasStride = _columns * _tileWidth;

        // Use fast-path block pointer if tile is contiguous in PROGMEM
        if (srcX == 0 && _columns == 1) {
            const uint16_t* tilePtr = _bitmap + (srcY * atlasStride);
            canvas->pushImage(posX, localY, _tileWidth, _tileHeight, tilePtr, transparentKey);
        } else {
            // High-speed row-by-row block copy (eliminates 1024 drawPixel calls per tile!)
            for (int y = 0; y < _tileHeight; y++) {
                int drawY = localY + y;
                if (drawY >= 0 && drawY < canvas->height()) {
                    const uint16_t* rowPtr = _bitmap + ((srcY + y) * atlasStride + srcX);
                    canvas->pushImage(posX, drawY, _tileWidth, 1, rowPtr, transparentKey);
                }
            }
        }
    }
};

#endif // TILESET_H
