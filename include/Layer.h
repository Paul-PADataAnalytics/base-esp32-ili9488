#ifndef LAYER_H
#define LAYER_H

#if defined(PLATFORM_LINUX) || !defined(ARDUINO)
#include "Arduino_Linux.h"
#else
#include <Arduino.h>
#endif
#include <LovyanGFX.hpp>
#include <functional>

class GFXContext;
class Layer;

/**
 * Layer Role Classification
 *
 * Roles determine strict z-ordering: BACKGROUND always renders below WORLD_MAP,
 * ENTITIES always above WORLD_MAP, UI_OVERLAY always on top.
 */
enum class LayerRole {
    BACKGROUND,   // Rendered first at lowest depth (sky, parallax)
    WORLD_MAP,    // Main level tilemaps
    ENTITIES,     // Game sprites & objects
    FOREGROUND,   // Above world & entities (fog, canopies, darkness masks)
    UI_OVERLAY    // Top-level HUD, score, menus
};

/**
 * Layer - Render Layer with Independent 2D Matrix Transforms & Dirty Tracking
 *
 * Each layer declares:
 *   - Its Y-range on the full canvas (minDrawY .. maxDrawY), so the engine
 *     knows which 80-px bands it occupies.
 *   - A dirty-state version counter (_stateVersion) incremented by the
 *     application whenever the layer's visual content changes. The engine
 *     compares this against a per-band "last seen" version to decide whether
 *     to re-render and re-push each band.
 *
 * The dirty mechanism is fully general: it does not encode any knowledge about
 * scroll positions, hero frames, or scene layout. The application updates the
 * version counter; the engine does the band → dirty mapping automatically.
 *
 * Usage:
 *   // In your app's update():
 *   if (scrollChanged) myLayer->markDirty();
 *
 *   // In LayerManager::computeBandDirtyFlags() (called by GFXContext::render):
 *   for each band:
 *     for each layer overlapping this band:
 *       if layer.stateVersion() != lastSeenVersion[layer][band]: band is dirty
 */
class Layer {
public:
    using RenderFunction = std::function<void(GFXContext& gfx, LGFX_Sprite* buffer, Layer& layer)>;

private:
    String         _name;
    LayerRole      _role;
    int            _zIndex;
    bool           _visible;

    // Y-range on the full canvas that this layer draws into.
    // Used by the engine to determine which bands this layer affects.
    int  _minDrawY;  // Inclusive. Default 0 (top of screen).
    int  _maxDrawY;  // Exclusive. Default 320 (full screen height).

    // Monotonically incrementing dirty counter. The engine compares this
    // against its own per-layer last-seen value per band. Increment to signal
    // that this layer's content has changed this frame.
    uint32_t _stateVersion;

    // Independent Layer Transform Properties
    float _translateX, _translateY;
    float _scaleX, _scaleY;
    float _rotationAngle;
    float _pivotX, _pivotY;

    RenderFunction _renderFunc;

public:
    Layer(const char* name, LayerRole role, int zIndex, RenderFunction renderFunc,
          int minDrawY = 0, int maxDrawY = 320)
        : _name(name), _role(role),
          _zIndex(calculateEffectiveZ(role, zIndex)), _visible(true),
          _minDrawY(minDrawY), _maxDrawY(maxDrawY),
          _stateVersion(1),        // Start at 1 so first frame is always dirty
          _translateX(0.0f), _translateY(0.0f),
          _scaleX(1.0f), _scaleY(1.0f),
          _rotationAngle(0.0f),
          _pivotX(240.0f), _pivotY(160.0f),
          _renderFunc(renderFunc) {}

    static int calculateEffectiveZ(LayerRole role, int userZ) {
        switch (role) {
            case LayerRole::BACKGROUND: return -1000 + userZ;
            case LayerRole::WORLD_MAP:  return     0 + userZ;
            case LayerRole::ENTITIES:   return  1000 + userZ;
            case LayerRole::FOREGROUND: return  5000 + userZ;
            case LayerRole::UI_OVERLAY: return 10000 + userZ;
            default: return userZ;
        }
    }

    // --- Identity & Visibility ---
    const char* getName()    const { return _name.c_str(); }
    LayerRole   getRole()    const { return _role; }
    int         getZIndex()  const { return _zIndex; }
    bool        isVisible()  const { return _visible; }
    void        setVisible(bool v) { _visible = v; }

    // --- Y-Range (used by engine for band/dirty mapping) ---
    int  getMinDrawY() const { return _minDrawY; }
    int  getMaxDrawY() const { return _maxDrawY; }
    void setDrawRange(int minY, int maxY) { _minDrawY = minY; _maxDrawY = maxY; }

    // --- Dirty State ---
    // Call markDirty() in update() whenever the layer's visual content changes.
    void     markDirty()          { _stateVersion++; }
    uint32_t stateVersion() const { return _stateVersion; }

    // --- Independent Layer Matrix Transforms ---
    void setTranslation(float x, float y) { _translateX = x; _translateY = y; }
    void move(float dx, float dy)         { _translateX += dx; _translateY += dy; }
    void setScale(float sx, float sy)     { _scaleX = sx; _scaleY = sy; }
    void setRotation(float angleDeg)      { _rotationAngle = angleDeg; }
    void rotate(float deltaDeg)           { _rotationAngle += deltaDeg; }
    void setPivot(float px, float py)     { _pivotX = px; _pivotY = py; }

    float getTranslationX()  const { return _translateX; }
    float getTranslationY()  const { return _translateY; }
    float getScaleX()        const { return _scaleX; }
    float getScaleY()        const { return _scaleY; }
    float getRotation()      const { return _rotationAngle; }

    // --- Rendering ---
    void render(GFXContext& gfx, LGFX_Sprite* buffer) {
        if (!_visible || !_renderFunc) return;
        _renderFunc(gfx, buffer, *this);
    }
};

#endif // LAYER_H
