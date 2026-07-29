#ifndef TILESET_H
#define TILESET_H

#include <Arduino.h>
#include <LovyanGFX.hpp>

/**
 * TileSet - Retro Graphics Atlas Module
 * 
 * Manages fixed-size tile spritesheets (8x8, 16x16) for retro games.
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

    /**
     * Renders a tile by index from the tileset atlas into a sprite buffer.
     */
    void drawTile(LGFX_Sprite* sprite, int tileIndex, int posX, int posY, uint16_t transparentKey = 0xFFFF) const {
        if (tileIndex < 0 || !_bitmap || !sprite) return;

        int srcX = (tileIndex % _columns) * _tileWidth;
        int srcY = (tileIndex / _columns) * _tileHeight;
        int atlasStride = _columns * _tileWidth;

        for (int y = 0; y < _tileHeight; y++) {
            for (int x = 0; x < _tileWidth; x++) {
                uint16_t pixel = pgm_read_word(&_bitmap[(srcY + y) * atlasStride + (srcX + x)]);
                if (pixel != transparentKey) {
                    sprite->drawPixel(posX + x, posY + y, pixel);
                }
            }
        }
    }
};

#endif // TILESET_H
