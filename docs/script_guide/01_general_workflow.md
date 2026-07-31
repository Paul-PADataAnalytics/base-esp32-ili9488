# Chapter 1: General Workflow

## 1.1 Architecture & Core Execution Flow

The engine uses an **Embedded Lua 5.4 State** bound to a high-performance C++ rendering and UI framework.

```
┌────────────────────────────────────────────────────────┐
│                      Lua 5.4 Script                    │
│    (UI Layout Creation, Event Handlers, Game Logic)     │
└───────────────────────────┬────────────────────────────┘
                            │ C C-API / Bridge
┌───────────────────────────▼────────────────────────────┐
│                       LuaEngine                        │
│   (State Allocation, C Functions, Interop Table)       │
└───────────────────────────┬────────────────────────────┘
                            │ Native C++ Calls
┌───────────────────────────▼────────────────────────────┐
│                 UIManager & UIWidgets                  │
│   (UIFrame, UIButton, UISlider, UILabel, Toast, etc.)   │
└───────────────────────────┬────────────────────────────┘
                            │ Dirty Region Tracking
┌───────────────────────────▼────────────────────────────┐
│             LayerManager & GFXContext                  │
│       (Banded Memory Rendering & Display Flush)        │
└────────────────────────────────────────────────────────┘
```

## 1.2 Memory & Allocation Model

* **Lua State Lifecycle**: Created on application initialization via `LuaEngine::init()`, which allocates a new `lua_State*` and registers standard libraries along with `ui.*` and `game.*` namespaces.
* **Widget Lifecycle**: Widgets instantiated from Lua script calls (such as `ui.createButton()`) are standard C++ heap objects stored inside an internal collection (`_allocatedWidgets`). Calling `ui.clear()` safely frees all active widgets and resets UI layout state without leaking memory.
* **Light Userdata References**: UI creation functions return a `lightuserdata` pointer pointing to the underlying `UIWidget*`. This handle can be stored in Lua global or local variables and passed into updater APIs like `ui.setText(widgetHandle, "New Text")`.

## 1.3 Event-Driven Callbacks & Rendering Pipeline

* **Asynchronous Callbacks**: When a user touches or interacts with a UI widget on the display (or via mouse/keyboard in the native Linux simulator), the C++ event listener routes the event to Lua by function name via `lua_pcall`.
* **Band-Based Dirty Rendering**: The engine renders scenes across horizontal bands (e.g., 400x80 slices on a 400x320 screen) to maintain a low RAM footprint on microcontrollers. When a Lua callback mutates UI state (e.g. changing text or moving a slider), `UIManager` marks itself dirty, triggering dirty-band redraws only for affected screen regions.
