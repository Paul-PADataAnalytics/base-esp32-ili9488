#ifndef LAYER_MANAGER_H
#define LAYER_MANAGER_H

#if defined(PLATFORM_LINUX) || !defined(ARDUINO)
#include "Arduino_Linux.h"
#else
#include <Arduino.h>
#endif
#include <LovyanGFX.hpp>
#include <vector>
#include <algorithm>
#include "Layer.h"

/**
 * LayerManager - Multi-Layer Compositing Engine with Generalised Band-Dirty Tracking
 *
 * ## Layer Compositing
 * Maintains layers in strict z-index depth order (BACKGROUND < WORLD_MAP <
 * ENTITIES < FOREGROUND < UI_OVERLAY). All rendering passes guarantee this order.
 *
 * ## Dirty-Rectangle Band Tracking (General)
 * The engine automatically determines which 80-px horizontal bands need
 * re-rendering each frame, without any scene-specific knowledge:
 *
 *   1. Each Layer declares its Y-range (minDrawY..maxDrawY).
 *   2. Each Layer maintains a stateVersion counter. Applications increment it
 *      by calling layer->markDirty() whenever visual content changes.
 *   3. LayerManager::computeBandDirtyFlags() compares each layer's current
 *      stateVersion against the last-seen version recorded per (layer, band).
 *      If any layer overlapping a band has a new version, that band is dirty.
 *   4. Only dirty bands are re-rendered and re-pushed via SPI DMA.
 *
 * This mechanism is completely scene-agnostic. It works for any number of
 * layers, any Y-layout, and any update frequency.
 */
class LayerManager {
public:
    static constexpr int MAX_BANDS  = 8;   // Supports up to 8 horizontal bands
    static constexpr int MAX_LAYERS = 16;  // Supports up to 16 layers

private:
    std::vector<Layer> _layers;

    // Per (layer-index, band-index) last-seen stateVersion.
    // Dirty if layer.stateVersion() != _lastSeenVersion[layerIdx][bandIdx].
    uint32_t _lastSeenVersion[MAX_LAYERS][MAX_BANDS];

public:
    LayerManager() {
        memset(_lastSeenVersion, 0, sizeof(_lastSeenVersion));
    }

    /**
     * Add a layer. Layers are kept sorted by z-index at all times.
     *
     * @param name        Human-readable debug name.
     * @param role        LayerRole enum (determines z-order slot).
     * @param userZIndex  Fine-grained z offset within the role slot.
     * @param renderFunc  Render callback — receives GFXContext, band sprite, and Layer ref.
     * @param minDrawY    Lowest canvas Y this layer ever draws to (inclusive). Default 0.
     * @param maxDrawY    Highest canvas Y this layer ever draws to (exclusive). Default 320.
     */
    void addLayer(const char* name, LayerRole role, int userZIndex,
                  Layer::RenderFunction renderFunc,
                  int minDrawY = 0, int maxDrawY = 320)
    {
        _layers.emplace_back(name, role, userZIndex, renderFunc, minDrawY, maxDrawY);
        std::sort(_layers.begin(), _layers.end(),
                  [](const Layer& a, const Layer& b) { return a.getZIndex() < b.getZIndex(); });
    }

    Layer* getLayer(const char* name) {
        for (auto& layer : _layers) {
            if (strcmp(layer.getName(), name) == 0) return &layer;
        }
        return nullptr;
    }

    void setLayerVisible(const char* name, bool visible) {
        Layer* l = getLayer(name);
        if (l) l->setVisible(visible);
    }

    void clearLayers() {
        _layers.clear();
        memset(_lastSeenVersion, 0, sizeof(_lastSeenVersion));
    }

    /**
     * Compute which bands need re-rendering this frame.
     *
     * For each band, checks all visible layers that overlap that band.
     * A band is dirty if any overlapping layer has a stateVersion newer
     * than the last value recorded by markBandClean().
     *
     * @param bandDirty  Output array, one bool per band. Must be size >= numBands.
     * @param numBands   Number of horizontal bands (e.g. 4 for 480x320 with 80px bands).
     * @param bandHeight Height in pixels of each band (e.g. 80).
     */
    void computeBandDirtyFlags(bool* bandDirty, int numBands, int bandHeight) const {
        for (int band = 0; band < numBands; band++) {
            int bandMinY = band * bandHeight;
            int bandMaxY = bandMinY + bandHeight;
            bandDirty[band] = false;

            for (int li = 0; li < (int)_layers.size() && li < MAX_LAYERS; li++) {
                const Layer& layer = _layers[li];
                if (!layer.isVisible()) continue;

                // Check if this layer's Y-range overlaps this band
                if (layer.getMinDrawY() >= bandMaxY) continue;
                if (layer.getMaxDrawY() <= bandMinY) continue;

                // Dirty if version has advanced since last seen
                if (layer.stateVersion() != _lastSeenVersion[li][band]) {
                    bandDirty[band] = true;
                    break;
                }
            }
        }
    }

    /**
     * Record that band[bandIdx] has been rendered with current layer versions.
     * Call after successfully pushing a band to the display.
     *
     * @param bandIdx   Which band was just rendered and pushed.
     * @param numBands  Total band count (guard).
     */
    void markBandClean(int bandIdx, int numBands) {
        if (bandIdx < 0 || bandIdx >= numBands) return;
        for (int li = 0; li < (int)_layers.size() && li < MAX_LAYERS; li++) {
            _lastSeenVersion[li][bandIdx] = _layers[li].stateVersion();
        }
    }

    /**
     * Composite all visible layers into the given sprite buffer.
     * Caller is responsible for setting gfx.setBandY() before calling this.
     */
    void renderAll(GFXContext& gfx, LGFX_Sprite* buffer) {
        for (auto& layer : _layers) {
            layer.render(gfx, buffer);
        }
    }
};

#endif // LAYER_MANAGER_H
