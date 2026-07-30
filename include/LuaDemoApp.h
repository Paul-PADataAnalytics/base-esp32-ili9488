#ifndef LUA_DEMO_APP_H
#define LUA_DEMO_APP_H

#include <Arduino.h>
#include "GFXContext.h"
#include "LayerManager.h"
#include "scripting/LuaEngine.h"
#include "ui/UIManager.h"

/**
 * LuaDemoApp — Comprehensive Sample Application Scripted Entirely via Lua
 *
 * Demonstrates:
 *   1. Full Lua 5.4 initialization & C++ binding integration
 *   2. Dynamic layout creation: frames, labels, buttons, checkboxes, sliders, option selectors
 *   3. Touch callbacks routing from UI components into Lua functions
 *   4. State variable management in Lua (`game.getVar`, `game.setVar`)
 *   5. Real-time dirty-rectangle rendering integration with `LayerManager`
 */
class LuaDemoApp {
private:
    LayerManager _layerManager;
    UIManager    _uiManager;
    LuaEngine    _lua;
    Layer*       _uiLayer = nullptr;

    const char* LUA_SCRIPT_DEMO = R"LUA(
        -- ===================================================================
        -- Lua 5.4 Choice RPG Adventure & UI Demonstration Script
        -- ===================================================================

        -- Global Game State
        game.setVar("gold", 50)
        game.setVar("hp", 100)
        game.setVar("volume", 0.75)
        game.setVar("hardcore", 0)

        function init_game_ui()
            ui.clear()
            ui.showToast("Lua 5.4 Engine Loaded!", 0x07FF, 3.0)

            -- Main Container Frame
            mainFrame = ui.createFrame(10, 10, 460, 300, "Lua RPG Quest Engine v1.0")

            -- Labels & Stats
            statusLabel = ui.createLabel(mainFrame, 15, 35, 430, 20, "Status: Exploring the Dark Cavern")
            statsLabel = ui.createLabel(mainFrame, 15, 60, 430, 20, "HP: 100 | Gold: 50")

            -- Option Selector for Class Choice
            ui.createOptionSelector(mainFrame, 15, 85, 430, 45, "Hero Class", {"Warrior", "Mage", "Rogue"}, "on_class_selected")

            -- Slider for Audio Volume
            ui.createSlider(mainFrame, 15, 135, 430, 45, "Audio Volume", 0.0, 1.0, 0.75, "on_volume_change")

            -- Checkbox for Hardcore Mode
            ui.createCheckBox(mainFrame, 15, 185, 430, 35, "Enable Hardcore Mode", false, "on_hardcore_toggle")

            -- Action Buttons
            ui.createButton(mainFrame, 15, 230, 200, 40, "Drink Potion (+20 HP)", "on_potion_click")
            ui.createButton(mainFrame, 245, 230, 200, 40, "Pay Innkeeper (10 Gold)", "on_inn_click")
        end

        -- Lua Callbacks
        function on_class_selected(idx)
            local classes = {"Warrior", "Mage", "Rogue"}
            local chosen = classes[idx + 1] or "Unknown"
            ui.showToast("Class Changed to: " .. chosen, 0x07FF, 2.0)
        end

        function on_volume_change(val)
            game.setVar("volume", val)
        end

        function on_hardcore_toggle(val)
            game.setVar("hardcore", val)
            if val == 1 then
                ui.showToast("Hardcore Mode Enabled!", 0xF800, 2.5)
            else
                ui.showToast("Normal Mode", 0x07FF, 1.5)
            end
        end

        function on_potion_click()
            local hp = game.getVar("hp") + 20
            if hp > 100 then hp = 100 end
            game.setVar("hp", hp)
            update_stats_display()
            ui.showToast("Healed +20 HP!", 0x07E0, 2.0)
        end

        function on_inn_click()
            local gold = game.getVar("gold")
            if gold >= 10 then
                gold = gold - 10
                game.setVar("gold", gold)
                update_stats_display()
                ui.showToast("Rested at Inn! (-10 Gold)", 0xFFE0, 2.0)
            else
                ui.showToast("Not enough Gold!", 0xF800, 2.0)
            end
        end

        function update_stats_display()
            local hp = game.getVar("hp")
            local gold = game.getVar("gold")
            -- Re-render stats banner
            ui.createLabel(mainFrame, 15, 60, 430, 20, "HP: " .. math.floor(hp) .. " | Gold: " .. math.floor(gold))
        end

        -- Initialize UI
        init_game_ui()
    )LUA";

public:
    LuaDemoApp() {}

    void setup(GFXContext& gfx) {
        _layerManager.clearLayers();
        _uiManager.setTheme(UITheme_Dark());

        // Initialize Lua engine & UI bindings
        if (!_lua.init(&_uiManager)) {
            Serial.println("[LUA DEMO ERROR] Failed to initialize Lua engine!");
            return;
        }

        // Register UI Layer with dirty-rectangle layer compositor
        _layerManager.addLayer("LuaUI", LayerRole::UI_OVERLAY, 0,
            [this](GFXContext& gfx, LGFX_Sprite* buf, Layer& layer) {
                _uiManager.draw(buf, gfx.getBandY(), layer);
            }, 0, 320);

        _uiLayer = _layerManager.getLayer("LuaUI");

        // Load & execute full Lua script
        _lua.executeString(LUA_SCRIPT_DEMO);
    }

    void update(float deltaTime, GFXContext& gfx) {
        _uiManager.update(deltaTime, gfx);
        if (_uiManager.isDirty() && _uiLayer) {
            _uiLayer->markDirty();
        }
    }

    void render(GFXContext& gfx) {
        bool bandDirty[LayerManager::MAX_BANDS] = {};
        _layerManager.computeBandDirtyFlags(bandDirty, gfx.getNumBands(), gfx.getBandHeight());
        gfx.renderBands(_layerManager, bandDirty);
    }
};

#endif // LUA_DEMO_APP_H
