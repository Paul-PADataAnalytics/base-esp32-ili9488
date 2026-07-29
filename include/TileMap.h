#ifndef TILEMAP_H
#define TILEMAP_H

#include "TileSet.h"

/**
 * TileMap - 2D Grid TileMap Module for Retro Games
 */
class TileMap {
private:
    const TileSet* _tileSet;
    const int16_t* _mapData;
    int            _mapWidth;  // Columns
    int            _mapHeight; // Rows

public:
    TileMap(const TileSet* tileSet, const int16_t* mapData, int mapW, int mapH)
        : _tileSet(tileSet), _mapData(mapData), _mapWidth(mapW), _mapHeight(mapH) {}

    int getMapWidth() const { return _mapWidth; }
    int getMapHeight() const { return _mapHeight; }

    int getTileAt(int col, int row) const {
        if (col < 0 || col >= _mapWidth || row < 0 || row >= _mapHeight || !_mapData) return -1;
        return _mapData[row * _mapWidth + col];
    }

    /**
     * Renders the tilemap onto a sprite buffer with offset support for scrolling.
     */
    void render(LGFX_Sprite* sprite, int offsetX = 0, int offsetY = 0, uint16_t transparentKey = 0xFFFF) const {
        if (!_tileSet || !_mapData || !sprite) return;

        int tw = _tileSet->getTileWidth();
        int th = _tileSet->getTileHeight();

        for (int row = 0; row < _mapHeight; row++) {
            for (int col = 0; col < _mapWidth; col++) {
                int tileId = _mapData[row * _mapWidth + col];
                if (tileId >= 0) {
                    int drawX = col * tw + offsetX;
                    int drawY = row * th + offsetY;

                    // Culling check: only render tiles visible in sprite viewport
                    if (drawX + tw >= 0 && drawX < sprite->width() &&
                        drawY + th >= 0 && drawY < sprite->height()) {
                        _tileSet->drawTile(sprite, tileId, drawX, drawY, transparentKey);
                    }
                }
            }
        }
    }
};

#endif // TILEMAP_H
