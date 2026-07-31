#ifndef GFX_CONTEXT_H
#define GFX_CONTEXT_H

#if defined(PLATFORM_LINUX) || !defined(ARDUINO)
#include "Arduino_Linux.h"
#include "hal/HALDisplay_Linux.h"
#else
#include <Arduino.h>
class HALDisplay_Linux;
#endif
#include <LovyanGFX.hpp>

class LayerManager;

/**
 * GFXContext - Hardware Abstraction Layer: Drawing, Input, Image & Power API
 */
class GFXContext {
private:
    lgfx::LGFX_Device* _lcd      = nullptr;
    HALDisplay_Linux*  _halLinux = nullptr;
    LGFX_Sprite*       _sprites[2];  // Double-buffer: two band-sized sprites
    int                _activeBuf;   // Index of sprite currently being rendered into
    float              _fps;

    int _canvasX;
    int _canvasY;
    int _canvasW;
    int _canvasH;
    int _bandY;       // Top Y-coordinate of the current band in canvas space
    int _bandHeight;  // Height in pixels of each band

public:
    GFXContext(lgfx::LGFX_Device* lcd,
               LGFX_Sprite* sprite0, LGFX_Sprite* sprite1,
               int cx, int cy, int cw, int ch,
               int bandH = 80)
        : _lcd(lcd), _activeBuf(0), _fps(0.0f),
          _canvasX(cx), _canvasY(cy), _canvasW(cw), _canvasH(ch),
          _bandY(0), _bandHeight(bandH)
    {
        _sprites[0] = sprite0;
        _sprites[1] = sprite1;
    }

    void setHALLinux(HALDisplay_Linux* hal) { _halLinux = hal; }
    HALDisplay_Linux* getHALLinux() const { return _halLinux; }

    // -------------------------------------------------------------------------
    // Engine State Accessors
    // -------------------------------------------------------------------------

    void  setFPS(float fps) { _fps = fps; }
    float getFPS()    const { return _fps; }

    int getWidth()      const { return _canvasW; }
    int getHeight()     const { return _canvasH; }
    int getBandHeight() const { return _bandHeight; }
    int getNumBands()   const { return _canvasH / _bandHeight; }
    int getSpriteX()    const { return _canvasX; }
    int getSpriteY()    const { return _canvasY; }
    int getSpriteW()    const { return _canvasW; }
    int getSpriteH()    const { return _canvasH; }

    int  getBandY()          const { return _bandY; }
    void setBandY(int bandY)       { _bandY = bandY; }

    lgfx::LGFX_Device* getLCD() { return _lcd; }

    LGFX_Sprite* getSprite() { return _sprites[_activeBuf]; }

    void renderBands(LayerManager& layerManager, const bool* bandDirty);

    // -------------------------------------------------------------------------
    // Display Power & Backlight Control
    // -------------------------------------------------------------------------
    void turnOnBacklight()  { digitalWrite(32, HIGH); }
    void turnOffBacklight() { digitalWrite(32, LOW); }
    void sleepDisplay()     { if (_lcd) _lcd->sleep(); }
    void wakeDisplay()      { if (_lcd) _lcd->wakeup(); }

    // -------------------------------------------------------------------------
    // Touch Screen / Mouse HAL API
    // -------------------------------------------------------------------------
    bool getTouch(int* x, int* y) {
#if defined(PLATFORM_LINUX) || !defined(ARDUINO)
        if (_halLinux) return _halLinux->getTouch(x, y);
        return false;
#else
        if (_lcd) return _lcd->getTouch(x, y);
        return false;
#endif
    }

    bool isTouched() {
#if defined(PLATFORM_LINUX) || !defined(ARDUINO)
        if (_halLinux) return _halLinux->isTouched();
        return false;
#else
        int x, y;
        return _lcd ? _lcd->getTouch(&x, &y) : false;
#endif
    }

    // -------------------------------------------------------------------------
    // HAL DMA Buffer Operations
    // -------------------------------------------------------------------------
    void pushBufferDMA() {
        LGFX_Sprite* s = _sprites[_activeBuf];
        if (s) {
#if defined(PLATFORM_LINUX) || !defined(ARDUINO)
            if (_halLinux) {
                _halLinux->updateFramebuffer(0, _bandY, s->width(), s->height(), (const uint16_t*)s->getBuffer());
            }
#else
            if (_lcd) {
                uint16_t* buf = (uint16_t*)s->getBuffer();
                _lcd->startWrite();
                _lcd->setAddrWindow(0, _bandY, s->width(), s->height());
                _lcd->writePixelsDMA(buf, s->width() * s->height());
            }
#endif
        }
    }

    void waitDMA() {
#if !defined(PLATFORM_LINUX) && defined(ARDUINO)
        if (_lcd) {
            _lcd->waitDMA();
            _lcd->endWrite();
        }
#endif
    }

    void swapBuffer() { _activeBuf ^= 1; }

    LGFX_Sprite* getPreviousSprite() { return _sprites[_activeBuf ^ 1]; }
};

// -------------------------------------------------------------------------
// renderBands() Implementation
// -------------------------------------------------------------------------
#include "LayerManager.h"

inline void GFXContext::renderBands(LayerManager& layerManager, const bool* bandDirty) {
    const int numBands = getNumBands();
    bool prevDirty = false;

    for (int band = 0; band < numBands; band++) {
        int bandY = band * _bandHeight;

        if (bandDirty[band]) {
            setBandY(bandY);
            layerManager.renderAll(*this, getSprite());

            if (prevDirty) waitDMA();
            pushBufferDMA();
            swapBuffer();
            layerManager.markBandClean(band, numBands);
            prevDirty = true;
        } else {
            if (prevDirty) {
                waitDMA();
                prevDirty = false;
            }
        }
    }

    if (prevDirty) waitDMA();
}

#endif // GFX_CONTEXT_H
