# Chapter 3: Packaging & Cross-Platform Support

## 3.1 Script Storage Strategies

Scripts can be packaged and delivered in two primary ways:

1. **Embedded String Literals (Zero File I/O Overhead)**:
   Embed raw Lua source text directly in C++ header/source files using raw string literals (`R"LUA(...)LUA"`). Recommended for built-in applications or standalone firmware builds.
   ```cpp
   const char* SCRIPT = R"LUA(
       ui.showToast("Embedded Script Loaded!", 0x07FF, 2.0)
   )LUA";
   _luaEngine.executeString(SCRIPT);
   ```

2. **Filesystem / Flash Storage (SPIFFS/LittleFS/SD Card)**:
   Read script files dynamically from disk or flash memory. Allows hot-reloading scripts without recompiling firmware.
   ```cpp
   File file = LittleFS.open("/game.lua", "r");
   String code = file.readString();
   _luaEngine.executeString(code.c_str());
   ```

## 3.2 Cross-Platform & ABI Compatibility

* **ESP32 Microcontroller Target**: Configured to compile embedded `Esp32Lua` sources linked against Arduino/FreeRTOS environments with strict memory constraints.
* **Linux Desktop Simulator**: Runs the exact same Lua script code natively inside an SDL2 window.
* **Floating-Point ABI Alignment**: Lua 5.4 in this engine uses single-precision 32-bit floating point numbers (`float`) across both Linux native builds and ESP32 targets to ensure bit-perfect numeric data exchange when calling `callLuaFunction()` and `game.setVar()`.
