#ifndef LUA_ENGINE_H
#define LUA_ENGINE_H

#if defined(PLATFORM_LINUX) || !defined(ARDUINO)
#include "Arduino_Linux.h"
#else
#include <Arduino.h>
#endif
#include <map>
#include <vector>

extern "C" {
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>
}

#include "../ui/UIManager.h"

/**
 * LuaEngine — Embedded Lua 5.4 Scripting & UI Binding Layer
 *
 * Exposes the UI framework to Lua scripts:
 *   - ui.createLabel(parent_frame, x, y, w, h, text)
 *   - ui.createButton(parent_frame, x, y, w, h, text, lua_func_name)
 *   - ui.createCheckBox(parent_frame, x, y, w, h, text, initial_val, lua_func_name)
 *   - ui.createSlider(parent_frame, x, y, w, h, text, min, max, initial_val, lua_func_name)
 *   - ui.createOptionSelector(parent_frame, x, y, w, h, text, options_tbl, lua_func_name)
 *   - ui.createFrame(x, y, w, h, title)
 *   - ui.showToast(msg, color_hex, duration)
 *   - ui.clear()
 *   - game.getVar(name), game.setVar(name, val)
 */
class LuaEngine {
private:
    lua_State* _luaState = nullptr;
    UIManager* _uiManager = nullptr;

    // Allocated UI widgets managed for Lua lifetime
    std::vector<UIWidget*> _allocatedWidgets;
    std::map<String, float> _gameVars;

    static LuaEngine* s_instance;

public:
    LuaEngine() {
        s_instance = this;
    }

    ~LuaEngine() {
        cleanup();
    }

    bool init(UIManager* ui) {
        _uiManager = ui;
        cleanup();

        _luaState = luaL_newstate();
        if (!_luaState) {
            Serial.println("[LUA ERROR] Failed to create Lua state!");
            return false;
        }

        luaL_openlibs(_luaState);
        registerBindings();

        Serial.println("[LUA ENGINE] Lua 5.4 state initialized successfully.");
        return true;
    }

    void cleanup() {
        if (_luaState) {
            lua_close(_luaState);
            _luaState = nullptr;
        }
        for (auto* w : _allocatedWidgets) {
            delete w;
        }
        _allocatedWidgets.clear();
    }

    bool executeString(const char* luaCode) {
        if (!_luaState) return false;
        int status = luaL_dostring(_luaState, luaCode);
        if (status != LUA_OK) {
            const char* errMsg = lua_tostring(_luaState, -1);
            Serial.printf("[LUA RUNTIME ERROR] %s\n", errMsg);
            if (_uiManager) {
                _uiManager->showToast("Lua Error!", 0xF800, 3.0f);
            }
            lua_pop(_luaState, 1);
            return false;
        }
        return true;
    }

    void callLuaFunction(const char* funcName, float arg = 0.0f) {
        if (!_luaState || !funcName || strlen(funcName) == 0) return;
        lua_getglobal(_luaState, funcName);
        if (lua_isfunction(_luaState, -1)) {
            lua_pushnumber(_luaState, arg);
            if (lua_pcall(_luaState, 1, 0, 0) != LUA_OK) {
                const char* errMsg = lua_tostring(_luaState, -1);
                Serial.printf("[LUA CALLBACK ERROR] '%s': %s\n", funcName, errMsg);
                lua_pop(_luaState, 1);
            }
        } else {
            lua_pop(_luaState, 1);
        }
    }

    void setGameVar(const char* name, float val) {
        _gameVars[name] = val;
        if (_luaState) {
            lua_pushnumber(_luaState, val);
            lua_setglobal(_luaState, name);
        }
    }

    float getGameVar(const char* name) {
        if (_gameVars.find(name) != _gameVars.end()) {
            return _gameVars[name];
        }
        return 0.0f;
    }

private:
    void registerBindings() {
        // Register 'ui' table functions
        lua_newtable(_luaState);

        lua_pushcfunction(_luaState, l_createFrame);
        lua_setfield(_luaState, -2, "createFrame");

        lua_pushcfunction(_luaState, l_createLabel);
        lua_setfield(_luaState, -2, "createLabel");

        lua_pushcfunction(_luaState, l_createButton);
        lua_setfield(_luaState, -2, "createButton");

        lua_pushcfunction(_luaState, l_createCheckBox);
        lua_setfield(_luaState, -2, "createCheckBox");

        lua_pushcfunction(_luaState, l_createSlider);
        lua_setfield(_luaState, -2, "createSlider");

        lua_pushcfunction(_luaState, l_createOptionSelector);
        lua_setfield(_luaState, -2, "createOptionSelector");

        lua_pushcfunction(_luaState, l_showToast);
        lua_setfield(_luaState, -2, "showToast");

        lua_pushcfunction(_luaState, l_clear);
        lua_setfield(_luaState, -2, "clear");

        lua_setglobal(_luaState, "ui");

        // Register 'game' table functions
        lua_newtable(_luaState);

        lua_pushcfunction(_luaState, l_getVar);
        lua_setfield(_luaState, -2, "getVar");

        lua_pushcfunction(_luaState, l_setVar);
        lua_setfield(_luaState, -2, "setVar");

        lua_setglobal(_luaState, "game");
    }

    // --- C Bridge Functions for Lua ---

    static int l_createFrame(lua_State* L) {
        int x = (int)luaL_checkinteger(L, 1);
        int y = (int)luaL_checkinteger(L, 2);
        int w = (int)luaL_checkinteger(L, 3);
        int h = (int)luaL_checkinteger(L, 4);
        const char* title = luaL_optstring(L, 5, "");

        UIFrame* frame = new UIFrame(x, y, w, h, title);
        s_instance->_allocatedWidgets.push_back(frame);
        s_instance->_uiManager->add(frame);

        lua_pushlightuserdata(L, frame);
        return 1;
    }

    static int l_createLabel(lua_State* L) {
        UIFrame* parent = (UIFrame*)lua_touserdata(L, 1);
        int x = (int)luaL_checkinteger(L, 2);
        int y = (int)luaL_checkinteger(L, 3);
        int w = (int)luaL_checkinteger(L, 4);
        int h = (int)luaL_checkinteger(L, 5);
        const char* text = luaL_checkstring(L, 6);

        UILabel* label = new UILabel(x, y, w, h, text);
        s_instance->_allocatedWidgets.push_back(label);

        if (parent) parent->add(label);
        else s_instance->_uiManager->add(label);

        lua_pushlightuserdata(L, label);
        return 1;
    }

    static int l_createButton(lua_State* L) {
        UIFrame* parent = (UIFrame*)lua_touserdata(L, 1);
        int x = (int)luaL_checkinteger(L, 2);
        int y = (int)luaL_checkinteger(L, 3);
        int w = (int)luaL_checkinteger(L, 4);
        int h = (int)luaL_checkinteger(L, 5);
        const char* labelStr = luaL_checkstring(L, 6);
        const char* funcName = luaL_optstring(L, 7, nullptr);

        UIButton* btn = new UIButton(x, y, w, h, labelStr);
        s_instance->_allocatedWidgets.push_back(btn);

        if (funcName && strlen(funcName) > 0) {
            String callbackName = funcName;
            btn->onPressed([callbackName]() {
                s_instance->callLuaFunction(callbackName.c_str());
            });
        }

        if (parent) parent->add(btn);
        else s_instance->_uiManager->add(btn);

        lua_pushlightuserdata(L, btn);
        return 1;
    }

    static int l_createCheckBox(lua_State* L) {
        UIFrame* parent = (UIFrame*)lua_touserdata(L, 1);
        int x = (int)luaL_checkinteger(L, 2);
        int y = (int)luaL_checkinteger(L, 3);
        int w = (int)luaL_checkinteger(L, 4);
        int h = (int)luaL_checkinteger(L, 5);
        const char* labelStr = luaL_checkstring(L, 6);
        bool initialVal = lua_toboolean(L, 7);
        const char* funcName = luaL_optstring(L, 8, nullptr);

        UICheckBox* chk = new UICheckBox(x, y, w, h, labelStr, initialVal);
        s_instance->_allocatedWidgets.push_back(chk);

        if (funcName && strlen(funcName) > 0) {
            String callbackName = funcName;
            chk->onChanged([callbackName](bool val) {
                s_instance->callLuaFunction(callbackName.c_str(), val ? 1.0f : 0.0f);
            });
        }

        if (parent) parent->add(chk);
        else s_instance->_uiManager->add(chk);

        lua_pushlightuserdata(L, chk);
        return 1;
    }

    static int l_createSlider(lua_State* L) {
        UIFrame* parent = (UIFrame*)lua_touserdata(L, 1);
        int x = (int)luaL_checkinteger(L, 2);
        int y = (int)luaL_checkinteger(L, 3);
        int w = (int)luaL_checkinteger(L, 4);
        int h = (int)luaL_checkinteger(L, 5);
        const char* labelStr = luaL_checkstring(L, 6);
        float minV = (float)luaL_checknumber(L, 7);
        float maxV = (float)luaL_checknumber(L, 8);
        float initV = (float)luaL_checknumber(L, 9);
        const char* funcName = luaL_optstring(L, 10, nullptr);

        UISlider* slider = new UISlider(x, y, w, h, labelStr, minV, maxV, initV);
        s_instance->_allocatedWidgets.push_back(slider);

        if (funcName && strlen(funcName) > 0) {
            String callbackName = funcName;
            slider->onChanged([callbackName](float val) {
                s_instance->callLuaFunction(callbackName.c_str(), val);
            });
        }

        if (parent) parent->add(slider);
        else s_instance->_uiManager->add(slider);

        lua_pushlightuserdata(L, slider);
        return 1;
    }

    static int l_createOptionSelector(lua_State* L) {
        UIFrame* parent = (UIFrame*)lua_touserdata(L, 1);
        int x = (int)luaL_checkinteger(L, 2);
        int y = (int)luaL_checkinteger(L, 3);
        int w = (int)luaL_checkinteger(L, 4);
        int h = (int)luaL_checkinteger(L, 5);
        const char* labelStr = luaL_checkstring(L, 6);

        UIOptionSelector* sel = new UIOptionSelector(x, y, w, h, labelStr);
        s_instance->_allocatedWidgets.push_back(sel);

        if (lua_istable(L, 7)) {
            int len = lua_rawlen(L, 7);
            for (int i = 1; i <= len; i++) {
                lua_rawgeti(L, 7, i);
                if (lua_isstring(L, -1)) {
                    sel->addOption(lua_tostring(L, -1));
                }
                lua_pop(L, 1);
            }
        }

        const char* funcName = luaL_optstring(L, 8, nullptr);
        if (funcName && strlen(funcName) > 0) {
            String callbackName = funcName;
            sel->onChanged([callbackName](int idx) {
                s_instance->callLuaFunction(callbackName.c_str(), (float)idx);
            });
        }

        if (parent) parent->add(sel);
        else s_instance->_uiManager->add(sel);

        lua_pushlightuserdata(L, sel);
        return 1;
    }

    static int l_showToast(lua_State* L) {
        const char* msg = luaL_checkstring(L, 1);
        uint16_t color = (uint16_t)luaL_optinteger(L, 2, 0x07FF);
        float duration = (float)luaL_optnumber(L, 3, 2.5f);

        s_instance->_uiManager->showToast(msg, color, duration);
        return 0;
    }

    static int l_clear(lua_State* L) {
        s_instance->_uiManager->clear();
        for (auto* w : s_instance->_allocatedWidgets) {
            delete w;
        }
        s_instance->_allocatedWidgets.clear();
        return 0;
    }

    static int l_getVar(lua_State* L) {
        const char* name = luaL_checkstring(L, 1);
        lua_pushnumber(L, s_instance->getGameVar(name));
        return 1;
    }

    static int l_setVar(lua_State* L) {
        const char* name = luaL_checkstring(L, 1);
        float val = (float)luaL_checknumber(L, 2);
        s_instance->setGameVar(name, val);
        return 0;
    }
};

// Static member definition
LuaEngine* LuaEngine::s_instance = nullptr;

#endif // LUA_ENGINE_H
