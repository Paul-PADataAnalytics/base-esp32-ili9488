# Chapter 2: Project Description

## 2.1 File & Project Structure

A standard game project using the Lua scripting engine consists of two main parts:
1. **Host C++ Application Harness**: Initializes display hardware, touch controllers, `UIManager`, `LayerManager`, and `LuaEngine`.
2. **Lua Game Script**: Defines scene layouts, game state variables, event handlers, and game loop updates.

```
project/
├── include/
│   ├── LuaEngine.h          # Core scripting engine bindings
│   ├── UIManager.h          # Native UI widget system
│   └── MyGameApp.h          # Custom C++ app wrapper
├── src/
│   └── main.cpp             # Entry point
└── data/
    └── scripts/
        └── game.lua         # Main Lua script file
```

## 2.2 State Management (`game.*`)

Game state parameters (player health, score, settings, audio volume) can be maintained either directly in Lua local/global variables or synced into the native host using `game.setVar(key, floatValue)` and `game.getVar(key)`.

Using `game.*` synchronizes state between C++ code and Lua scripts, allowing native systems (such as high-level application managers or save systems) to observe game metrics.

## 2.3 Component Tree & Spatial Positioning

Widgets follow a hierarchical container model:
* **Root Canvas**: Submitting `nil` or omitting the parent parameter attaches widgets directly to the global `UIManager` root space.
* **Containers (`UIFrame`)**: Creating a `UIFrame` provides a titled dialog container. Passing the frame handle into widget creation functions places widgets relative to the frame boundaries.
