#ifndef LAYER_MANAGER_H
#define LAYER_MANAGER_H

#include <Arduino.h>
#include <LovyanGFX.hpp>
#include <vector>
#include <algorithm>
#include "Layer.h"

/**
 * LayerManager - Multi-Layer Compositing Engine
 * 
 * Manages Background, TileMap, Entity Sprites, Foreground, and UI layers.
 * Guarantees Background layers ALWAYS render below everything else and
 * Foreground layers ALWAYS render above everything else.
 */
class LayerManager {
private:
    std::vector<Layer> _layers;

public:
    LayerManager() {}

    /**
     * Add a layer specifying role and sub-z-index offset.
     */
    void addLayer(const char* name, LayerRole role, int userZIndex, Layer::RenderFunction renderFunc) {
        _layers.emplace_back(name, role, userZIndex, renderFunc);
        std::sort(_layers.begin(), _layers.end(), [](const Layer& a, const Layer& b) {
            return a.getZIndex() < b.getZIndex();
        });
    }

    Layer* getLayer(const char* name) {
        for (auto& layer : _layers) {
            if (strcmp(layer.getName(), name) == 0) {
                return &layer;
            }
        }
        return nullptr;
    }

    void setLayerVisible(const char* name, bool visible) {
        Layer* l = getLayer(name);
        if (l) l->setVisible(visible);
    }

    void clearLayers() {
        _layers.clear();
    }

    /**
     * Composite all layers in strict z-index depth order.
     */
    void renderAll(GFXContext& gfx, LGFX_Sprite* buffer) {
        for (auto& layer : _layers) {
            layer.render(gfx, buffer);
        }
    }
};

#endif // LAYER_MANAGER_H
