# Chapter 4: Putting It All Together (Complete Working Tutorial)

This chapter provides a complete end-to-end working example: a RPG Inventory & Shop game script.

## 4.1 Step 1: Write the Lua Game Script (`game.lua`)

```lua
-- ===================================================================
-- RPG Shop & Inventory Script
-- ===================================================================

-- Initial Game Variables
game.setVar("gold", 150)
game.setVar("potions", 2)
game.setVar("sword_level", 1)

function build_shop_ui()
    -- Clear any existing UI widgets
    ui.clear()
    ui.showToast("Welcome to the Item Shop!", 0x07FF, 2.5)

    -- Main Container Window (x, y, w, h, title)
    shopFrame = ui.createFrame(10, 10, 460, 300, "Village Adventurer Shop")

    -- Inventory & Gold Displays
    goldLabel = ui.createLabel(shopFrame, 20, 35, 420, 20, "Gold: 150")
    invLabel  = ui.createLabel(shopFrame, 20, 60, 420, 20, "Potions: 2 | Sword Lvl: 1")

    -- Difficulty Option Selector
    ui.createOptionSelector(shopFrame, 20, 90, 420, 40, "Difficulty", {"Easy", "Normal", "Hard"}, "on_difficulty_change")

    -- Music Volume Slider
    ui.createSlider(shopFrame, 20, 135, 420, 40, "BGM Volume", 0.0, 1.0, 0.8, "on_volume_change")

    -- Hardcore Toggle Checkbox
    ui.createCheckBox(shopFrame, 20, 180, 420, 30, "Enable Permadeath", false, "on_permadeath_toggle")

    -- Action Buttons
    ui.createButton(shopFrame, 20, 225, 200, 40, "Buy Potion (25 Gold)", "buy_potion")
    ui.createButton(shopFrame, 240, 225, 200, 40, "Upgrade Sword (50 Gold)", "upgrade_sword")
end

-- Callback Handlers
function buy_potion()
    local gold = game.getVar("gold")
    if gold >= 25 then
        gold = gold - 25
        local potions = game.getVar("potions") + 1
        game.setVar("gold", gold)
        game.setVar("potions", potions)
        
        refresh_display()
        ui.showToast("Purchased 1 Potion!", 0x07E0, 1.5)
    else
        ui.showToast("Not enough gold!", 0xF800, 2.0)
    end
end

function upgrade_sword()
    local gold = game.getVar("gold")
    if gold >= 50 then
        gold = gold - 50
        local lvl = game.getVar("sword_level") + 1
        game.setVar("gold", gold)
        game.setVar("sword_level", lvl)

        refresh_display()
        ui.showToast("Sword Upgraded to Level " .. math.floor(lvl) .. "!", 0xFFE0, 2.0)
    else
        ui.showToast("Need 50 Gold for upgrade!", 0xF800, 2.0)
    end
end

function on_difficulty_change(idx)
    local diffs = {"Easy", "Normal", "Hard"}
    local name = diffs[idx + 1] or "Normal"
    ui.showToast("Difficulty set to: " .. name, 0x07FF, 1.5)
end

function on_volume_change(val)
    game.setVar("bgm_volume", val)
end

function on_permadeath_toggle(val)
    if val == 1 then
        ui.showToast("Permadeath active! Be careful!", 0xF800, 2.5)
    end
end

function refresh_display()
    local g = math.floor(game.getVar("gold"))
    local p = math.floor(game.getVar("potions"))
    local s = math.floor(game.getVar("sword_level"))
    
    ui.setText(goldLabel, "Gold: " .. g)
    ui.setText(invLabel, "Potions: " .. p .. " | Sword Lvl: " .. s)
end

-- Initialize shop
build_shop_ui()
```

## 4.2 Step 2: C++ Application Integration Harness

```cpp
#include "GFXContext.h"
#include "LayerManager.h"
#include "ui/UIManager.h"
#include "scripting/LuaEngine.h"

class ShopApp {
private:
    LayerManager _layerManager;
    UIManager    _uiManager;
    LuaEngine    _luaEngine;
    Layer*       _uiLayer = nullptr;

public:
    void setup(GFXContext& gfx) {
        _layerManager.clearLayers();
        _uiManager.setTheme(UITheme_Dark());

        // 1. Initialize Lua Engine with UI manager reference
        if (!_luaEngine.init(&_uiManager)) {
            Serial.println("[ERROR] Lua Engine Failed to Initialize!");
            return;
        }

        // 2. Register UI layer with dirty-band rendering compositor
        _layerManager.addLayer("ShopUI", LayerRole::UI_OVERLAY, 0,
            [this](GFXContext& gfx, LGFX_Sprite* buf, Layer& layer) {
                _uiManager.draw(buf, gfx.getBandY(), layer);
            }, 0, 320);

        _uiLayer = _layerManager.getLayer("ShopUI");

        // 3. Load & run the Lua script file or string
        const char* scriptCode = /* raw string from game.lua */;
        _luaEngine.executeString(scriptCode);
    }

    void update(float deltaTime, GFXContext& gfx) {
        // Update touch states and UI animations
        _uiManager.update(deltaTime, gfx);
        
        // Mark layer dirty if any widget changed
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
```
