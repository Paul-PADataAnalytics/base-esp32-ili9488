#ifndef GFX_CONTEXT_H
#define GFX_CONTEXT_H

#include <Arduino.h>
#include <LovyanGFX.hpp>

// Forward declare to avoid circular include (LayerManager includes GFXContext,
// GFXContext's renderBands() takes a LayerManager reference).
class LayerManager;

/**
 * GFXContext - Hardware Abstraction Layer: Drawing, Input, Image & Power API
 *
 * ## Overview
 * GFXContext is the single point of contact between application/engine code
 * and the physical display hardware. It completely hides:
 *   - SPI bus management (startWrite / endWrite / waitDMA)
 *   - Band-buffer geometry (which sprite is active, current bandY offset)
 *   - DMA double-buffering (two sprite buffers alternated each band pass)
 *
 * ## Band Rendering
 * The display (480×320) is split into N horizontal bands of equal height
 * (default: 4 bands × 80 px). Each band is rendered into a small sprite
 * buffer in DRAM and pushed to the display via SPI DMA.
 *
 * ## DMA Double-Buffering (General)
 * Two sprite buffers are allocated. While band N's DMA transfer runs in
 * hardware, the CPU renders band N+1 into the idle buffer. The buffers
 * are swapped between bands. This hides most of the SPI transfer latency.
 *
 * ## Dirty-Rectangle Integration (General)
 * renderBands() accepts a LayerManager and a pre-computed bandDirty[] array.
 * Bands flagged as clean are skipped entirely — no re-render, no SPI push.
 * After a band is pushed, LayerManager::markBandClean() is called so the
 * dirty state remains consistent for the next frame.
 *
 * ## HAL Note
 * To port to Linux/SDL for desktop development, implement a GFXContext
 * subclass (or compile-time swap) that replaces pushBufferDMA() / waitDMA()
 * with SDL_UpdateTexture / SDL_RenderPresent calls. All application and
 * engine code above this layer requires zero changes.
 */
class GFXContext {
private:
    lgfx::LGFX_Device* _lcd;
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
    /**
     * Construct a GFXContext.
     *
     * @param lcd      Pointer to the LovyanGFX display device.
     * @param sprite0  First band sprite buffer (slot A of double-buffer).
     * @param sprite1  Second band sprite buffer (slot B of double-buffer).
     * @param cx,cy    Canvas top-left origin on the display (usually 0,0).
     * @param cw,ch    Canvas dimensions in pixels (e.g. 480, 320).
     * @param bandH    Height of each horizontal band (e.g. 80). Must divide ch evenly.
     */
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

    /** Returns the sprite currently being rendered into. */
    LGFX_Sprite* getSprite() { return _sprites[_activeBuf]; }

    // -------------------------------------------------------------------------
    // General DMA Double-Buffer Render Loop
    //
    // renderBands() is the canonical, scene-agnostic frame render entry point.
    // Call it once per frame from your application's render() method.
    //
    // It implements the following pipeline per band:
    //   1. Skip band entirely if bandDirty[band] == false.
    //   2. While previous band's DMA transfer runs in hardware:
    //      - CPU renders current band into the idle sprite buffer.
    //   3. Wait for previous DMA to finish.
    //   4. Initiate DMA for current band (non-blocking).
    //   5. Swap buffers so next render targets the now-idle buffer.
    //   6. Call layerManager.markBandClean(band) to update dirty tracking.
    //
    // @param layerManager  Reference to the scene's LayerManager.
    // @param bandDirty     Array of bool, one per band (from computeBandDirtyFlags()).
    // -------------------------------------------------------------------------
    void renderBands(LayerManager& layerManager, const bool* bandDirty);

    // -------------------------------------------------------------------------
    // Display Power & Backlight Control
    // -------------------------------------------------------------------------
    void turnOnBacklight()  { digitalWrite(32, HIGH); }
    void turnOffBacklight() { digitalWrite(32, LOW); }
    void sleepDisplay()     { _lcd->sleep(); }
    void wakeDisplay()      { _lcd->wakeup(); }

    // -------------------------------------------------------------------------
    // Touch Screen API
    // -------------------------------------------------------------------------
    bool getTouch(int* x, int* y) { return _lcd->getTouch(x, y); }
    bool isTouched() { int x, y; return _lcd->getTouch(&x, &y); }

    // -------------------------------------------------------------------------
    // Color Utilities
    // -------------------------------------------------------------------------
    uint16_t color565(uint8_t r, uint8_t g, uint8_t b) {
        return _lcd->color565(r, g, b);
    }

    uint16_t blendColor(uint16_t c1, uint16_t c2, uint8_t ratio) {
        uint8_t r1 = (c1 >> 11) & 0x1F; uint8_t g1 = (c1 >> 5) & 0x3F; uint8_t b1 = c1 & 0x1F;
        uint8_t r2 = (c2 >> 11) & 0x1F; uint8_t g2 = (c2 >> 5) & 0x3F; uint8_t b2 = c2 & 0x1F;
        return ((uint16_t)((r1*(255-ratio)+r2*ratio)>>8) << 11)
             | ((uint16_t)((g1*(255-ratio)+g2*ratio)>>8) << 5)
             |  (uint16_t)((b1*(255-ratio)+b2*ratio)>>8);
    }

    // -------------------------------------------------------------------------
    // Drawing Primitives (band-local coordinate translation applied automatically)
    // -------------------------------------------------------------------------
    void fillScreen(uint16_t color) {
        LGFX_Sprite* s = _sprites[_activeBuf];
        if (s) s->fillScreen(color); else _lcd->fillScreen(color);
    }

    void pushImageDirect(int x, int y, int w, int h, const uint16_t* data) {
        LGFX_Sprite* s = _sprites[_activeBuf];
        if (s) s->pushImage(x, y - _bandY, w, h, data);
        else   _lcd->pushImage(x, y, w, h, data);
    }

    void pushImageTransparent(int x, int y, int w, int h, const uint16_t* data, uint16_t tColor) {
        LGFX_Sprite* s = _sprites[_activeBuf];
        if (s) s->pushImage(x, y - _bandY, w, h, data, tColor);
        else   _lcd->pushImage(x, y, w, h, data, tColor);
    }

    void drawCircleDirect(int x, int y, int r, uint16_t color, bool fill = true) {
        LGFX_Sprite* s = _sprites[_activeBuf];
        int ly = y - _bandY;
        if (ly + r < 0 || ly - r >= _bandHeight) return;
        if (s) { if (fill) s->fillCircle(x, ly, r, color); else s->drawCircle(x, ly, r, color); }
        else   { if (fill) _lcd->fillCircle(x, y, r, color); else _lcd->drawCircle(x, y, r, color); }
    }

    void eraseCircleDirect(int x, int y, int r, uint16_t color) {
        LGFX_Sprite* s = _sprites[_activeBuf];
        int ly = y - _bandY;
        if (ly + r < 0 || ly - r >= _bandHeight) return;
        if (s) s->fillCircle(x, ly, r + 2, color);
        else   _lcd->fillCircle(x, y, r + 2, color);
    }

    void drawRectDirect(int x, int y, int w, int h, uint16_t color) {
        LGFX_Sprite* s = _sprites[_activeBuf];
        int ly = y - _bandY;
        if (ly + h < 0 || ly >= _bandHeight) return;
        if (s) s->drawRect(x, ly, w, h, color); else _lcd->drawRect(x, y, w, h, color);
    }

    void fillRectDirect(int x, int y, int w, int h, uint16_t color) {
        LGFX_Sprite* s = _sprites[_activeBuf];
        int ly = y - _bandY;
        if (ly + h < 0 || ly >= _bandHeight) return;
        if (s) s->fillRect(x, ly, w, h, color); else _lcd->fillRect(x, y, w, h, color);
    }

    void fillRoundRectDirect(int x, int y, int w, int h, int r, uint16_t color) {
        LGFX_Sprite* s = _sprites[_activeBuf];
        int ly = y - _bandY;
        if (ly + h < 0 || ly >= _bandHeight) return;
        if (s) s->fillRoundRect(x, ly, w, h, r, color); else _lcd->fillRoundRect(x, y, w, h, r, color);
    }

    void drawRoundRectDirect(int x, int y, int w, int h, int r, uint16_t color) {
        LGFX_Sprite* s = _sprites[_activeBuf];
        int ly = y - _bandY;
        if (ly + h < 0 || ly >= _bandHeight) return;
        if (s) s->drawRoundRect(x, ly, w, h, r, color); else _lcd->drawRoundRect(x, y, w, h, r, color);
    }

    void drawLineDirect(int x1, int y1, int x2, int y2, uint16_t color) {
        LGFX_Sprite* s = _sprites[_activeBuf];
        if (s) s->drawLine(x1, y1 - _bandY, x2, y2 - _bandY, color);
        else   _lcd->drawLine(x1, y1, x2, y2, color);
    }

    void drawTextDirect(const char* text, int x, int y, uint16_t color,
                        uint8_t size = 2, uint16_t bgColor = 0x0813) {
        LGFX_Sprite* s = _sprites[_activeBuf];
        int ly = y - _bandY;
        if (ly + 8 * size < 0 || ly >= _bandHeight) return;
        if (s) { s->setTextColor(color, bgColor); s->setTextSize(size); s->setCursor(x, ly); s->print(text); }
        else   { _lcd->setTextColor(color, bgColor); _lcd->setTextSize(size); _lcd->setCursor(x, y); _lcd->print(text); }
    }

    // -------------------------------------------------------------------------
    // Low-Level Band Push (for custom render loops; prefer renderBands())
    // -------------------------------------------------------------------------

    /** Blocking push of active sprite to display at current bandY. */
    void pushBuffer() {
        LGFX_Sprite* s = _sprites[_activeBuf];
        if (s) s->pushSprite(_lcd, 0, _bandY);
    }

    /**
     * Non-blocking DMA push of active sprite to display at current bandY.
     * Must call waitDMA() before the next pushBufferDMA() or touching the SPI bus.
     */
    void pushBufferDMA() {
        LGFX_Sprite* s = _sprites[_activeBuf];
        if (s) {
            uint16_t* buf = (uint16_t*)s->getBuffer();
            _lcd->startWrite();
            _lcd->setAddrWindow(0, _bandY, s->width(), s->height());
            _lcd->writePixelsDMA(buf, s->width() * s->height());
        }
    }

    /** Wait for any in-flight DMA transfer to complete and release the SPI bus. */
    void waitDMA() {
        _lcd->waitDMA();
        _lcd->endWrite();
    }

    /** Swap active and idle sprite buffers. */
    void swapBuffer() { _activeBuf ^= 1; }

    /** Returns the sprite that was just rendered (not currently being written into). */
    LGFX_Sprite* getPreviousSprite() { return _sprites[_activeBuf ^ 1]; }
};

// -------------------------------------------------------------------------
// renderBands() Implementation
// Defined here (not in a .cpp) to keep the project header-only on ESP32.
// Include LayerManager.h after GFXContext.h.
// -------------------------------------------------------------------------
#include "LayerManager.h"

inline void GFXContext::renderBands(LayerManager& layerManager, const bool* bandDirty) {
    const int numBands = getNumBands();
    bool prevDirty = false;

    for (int band = 0; band < numBands; band++) {
        int bandY = band * _bandHeight;

        if (bandDirty[band]) {
            // CPU renders into active buffer.
            // If previous band's DMA is still running, it runs concurrently here.
            setBandY(bandY);
            layerManager.renderAll(*this, getSprite());

            // Gate: must not start new DMA until previous is done.
            if (prevDirty) waitDMA();

            // Push this band non-blocking via DMA.
            pushBufferDMA();

            // Swap so next render targets the now-idle buffer.
            swapBuffer();

            // Record that this band has been rendered with current layer versions.
            layerManager.markBandClean(band, numBands);

            prevDirty = true;
        } else {
            // Band is clean — content unchanged. Flush any pending DMA and skip.
            if (prevDirty) {
                waitDMA();
                prevDirty = false;
            }
        }
    }

    // Final flush for the last in-flight DMA.
    if (prevDirty) waitDMA();
}

#endif // GFX_CONTEXT_H
