/**
 * ESP32 Hardware-Abstracted Lua UI Game Benchmark & Framework Demonstration
 */

#include <Arduino.h>
#include <LovyanGFX.hpp>
#include "GFXContext.h"
#include "LuaDemoApp.h"
#include "DisplayDriver.h"

// Instantiate Global Drivers
static LGFX_ILI9488 tft;
static LGFX_Sprite  spriteBuffer0(&tft);   // Band buffer A
static LGFX_Sprite  spriteBuffer1(&tft);   // Band buffer B

const int CANVAS_W = 480;
const int CANVAS_H = 320;
const int BAND_H   = 80;

static GFXContext gfx(&tft, &spriteBuffer0, &spriteBuffer1, 0, 0, CANVAS_W, CANVAS_H);
static LuaDemoApp myApp;

unsigned long lastFrameTime = 0;
unsigned long lastFPSTime   = 0;
int           frameCount    = 0;

void setup() {
    pinMode(32, OUTPUT);
    digitalWrite(32, HIGH); // Backlight HIGH

    pinMode(33, OUTPUT);
    digitalWrite(33, HIGH); // Pull TCS HIGH

    Serial.begin(115200);
    delay(300);
    Serial.println("\n[BENCHMARK] Initializing Embedded Lua 5.4 UI Engine...");
    Serial.printf("[BENCHMARK MEMORY] Free Heap: %d bytes, Max Alloc: %d bytes\n", ESP.getFreeHeap(), ESP.getMaxAllocHeap());

    tft.init();
    tft.setRotation(1); // Landscape mode (480x320)
    tft.fillScreen(tft.color565(135, 206, 235));

    spriteBuffer0.setColorDepth(16);
    spriteBuffer0.createSprite(CANVAS_W, BAND_H);

    spriteBuffer1.setColorDepth(16);
    spriteBuffer1.createSprite(CANVAS_W, BAND_H);

    Serial.printf("[BENCHMARK MEMORY] After buffers - Free Heap: %d bytes\n", ESP.getFreeHeap());

    // Initialize application & Lua script environment
    myApp.setup(gfx);

    lastFrameTime = millis();
    lastFPSTime   = millis();
    Serial.println("[BENCHMARK] Setup Complete. Running Lua 5.4 Choice Engine.");
}

void loop() {
    unsigned long now = millis();
    float deltaTime = (now - lastFrameTime) / 1000.0f;
    if (deltaTime <= 0.0f) deltaTime = 0.016f;
    lastFrameTime = now;

    // 1. Application logic & touch routing
    myApp.update(deltaTime, gfx);

    // 2. Render pass
    unsigned long renderStart = micros();
    myApp.render(gfx);
    unsigned long renderUs = micros() - renderStart;

    // 3. Metrics reporting
    frameCount++;
    if (now - lastFPSTime >= 1000) {
        float fps = frameCount * 1000.0f / (now - lastFPSTime);
        gfx.setFPS(fps);
        lastFPSTime = now;
        frameCount = 0;
        Serial.printf("[LUA ENGINE METRICS] FPS: %.1f | Render: %lu us | Free Heap: %d bytes\n",
                      fps, renderUs, ESP.getFreeHeap());
    }
}
