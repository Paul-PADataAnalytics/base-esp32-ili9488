#ifndef LAYER_H
#define LAYER_H

#include <Arduino.h>
#include <LovyanGFX.hpp>
#include <functional>

class GFXContext;
class Layer;

/**
 * Layer Role Classification
 */
enum class LayerRole {
    BACKGROUND,   // Always rendered FIRST at lowest depth (below everything)
    WORLD_MAP,    // Main level tilemaps
    ENTITIES,     // Game sprites & objects
    FOREGROUND,   // Always rendered ABOVE world & entities (tree canopies, darkness masks)
    UI_OVERLAY    // Top-level UI, score, HUD
};

/**
 * Layer - Render Layer with Independent 2D Matrix Transforms
 * 
 * Supports independent translation (scroll), scale, rotation, and pivot points per layer!
 */
class Layer {
public:
    using RenderFunction = std::function<void(GFXContext& gfx, LGFX_Sprite* buffer, Layer& layer)>;

private:
    String         _name;
    LayerRole      _role;
    int            _zIndex;
    bool           _visible;

    // Independent Layer Transform Properties
    float _translateX, _translateY;
    float _scaleX, _scaleY;
    float _rotationAngle; // Degrees
    float _pivotX, _pivotY;

    RenderFunction _renderFunc;

public:
    Layer(const char* name, LayerRole role, int zIndex, RenderFunction renderFunc)
        : _name(name), _role(role), _zIndex(calculateEffectiveZ(role, zIndex)), _visible(true),
          _translateX(0.0f), _translateY(0.0f),
          _scaleX(1.0f), _scaleY(1.0f),
          _rotationAngle(0.0f),
          _pivotX(240.0f), _pivotY(160.0f),
          _renderFunc(renderFunc) {}

    static int calculateEffectiveZ(LayerRole role, int userZ) {
        switch (role) {
            case LayerRole::BACKGROUND: return -1000 + userZ;
            case LayerRole::WORLD_MAP:  return 0 + userZ;
            case LayerRole::ENTITIES:   return 1000 + userZ;
            case LayerRole::FOREGROUND: return 5000 + userZ;
            case LayerRole::UI_OVERLAY: return 10000 + userZ;
            default: return userZ;
        }
    }

    const char* getName() const { return _name.c_str(); }
    LayerRole getRole() const { return _role; }
    int getZIndex() const { return _zIndex; }
    bool isVisible() const { return _visible; }
    void setVisible(bool v) { _visible = v; }

    // Independent Layer Matrix Transform Setters
    void setTranslation(float x, float y) { _translateX = x; _translateY = y; }
    void move(float dx, float dy) { _translateX += dx; _translateY += dy; }
    void setScale(float sx, float sy) { _scaleX = sx; _scaleY = sy; }
    void setRotation(float angleDeg) { _rotationAngle = angleDeg; }
    void rotate(float deltaDeg) { _rotationAngle += deltaDeg; }
    void setPivot(float px, float py) { _pivotX = px; _pivotY = py; }

    float getTranslationX() const { return _translateX; }
    float getTranslationY() const { return _translateY; }
    float getScaleX() const { return _scaleX; }
    float getScaleY() const { return _scaleY; }
    float getRotation() const { return _rotationAngle; }

    /**
     * Render the layer contents
     */
    void render(GFXContext& gfx, LGFX_Sprite* buffer) {
        if (!_visible || !_renderFunc || !buffer) return;
        _renderFunc(gfx, buffer, *this);
    }
};

#endif // LAYER_H
