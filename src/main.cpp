/**
 * ESP32 Hardware-Abstracted Side-Scroller Platformer Benchmark
 */

#include <Arduino.h>
#include <LovyanGFX.hpp>
#include "GFXContext.h"
#include "SideScrollerApp.h"
#include "DisplayDriver.h"

// Instantiate Global Drivers
static LGFX_ILI9488 tft;
static LGFX_Sprite  spriteBuffer0(&tft);   // Band buffer A (DMA double-buffer slot 0)
static LGFX_Sprite  spriteBuffer1(&tft);   // Band buffer B (DMA double-buffer slot 1)

// Native 480x320 Viewport & 480x80 Band Buffer (76.8 KB RAM each)
const int CANVAS_W = 480;
const int CANVAS_H = 320;
const int BAND_H   = 80;

static GFXContext gfx(&tft, &spriteBuffer0, &spriteBuffer1, 0, 0, CANVAS_W, CANVAS_H);
static SideScrollerApp myApp;

unsigned long lastFrameTime = 0;
unsigned long lastFPSTime   = 0;
int           frameCount    = 0;

void setup() {
    // 1. Force Backlight Pin HIGH (GPIO 32) & Deselect Touch CS (GPIO 33)
    pinMode(32, OUTPUT);
    digitalWrite(32, HIGH); // Backlight HIGH (Turn on LED)

    pinMode(33, OUTPUT);
    digitalWrite(33, HIGH); // Pull TCS HIGH (deselected)

    Serial.begin(115200);
    delay(300);
    Serial.println("\n[BENCHMARK] Initializing Realistic Side-Scroller Platformer Benchmark...");
    Serial.printf("[BENCHMARK MEMORY] Free Heap: %d bytes, Max Alloc: %d bytes\n", ESP.getFreeHeap(), ESP.getMaxAllocHeap());

    // 2. Initialize Display Driver
    tft.init();
    tft.setRotation(1); // Landscape mode (480x320 native resolution)
    tft.fillScreen(tft.color565(135, 206, 235));

    // Allocate TWO 480x80 16-bit RGB565 band buffers for DMA double-buffering
    spriteBuffer0.setColorDepth(16);
    void* ptr0 = spriteBuffer0.createSprite(CANVAS_W, BAND_H);
    if (ptr0) {
        Serial.printf("[BENCHMARK] SUCCESS: Band buffer A allocated (%d bytes).\n", CANVAS_W * BAND_H * 2);
    } else {
        Serial.println("[BENCHMARK ERROR] Band buffer A allocation FAILED!");
    }

    spriteBuffer1.setColorDepth(16);
    void* ptr1 = spriteBuffer1.createSprite(CANVAS_W, BAND_H);
    if (ptr1) {
        Serial.printf("[BENCHMARK] SUCCESS: Band buffer B allocated (%d bytes).\n", CANVAS_W * BAND_H * 2);
    } else {
        Serial.println("[BENCHMARK ERROR] Band buffer B allocation FAILED! Falling back to single-buffer.");
    }

    Serial.printf("[BENCHMARK MEMORY] After buffers - Free Heap: %d bytes\n", ESP.getFreeHeap());

    // 3. Initialize application using high-level drawing context
    myApp.setup(gfx);

    lastFrameTime = millis();
    lastFPSTime   = millis();
    Serial.println("[BENCHMARK] Setup Complete. Running Side-Scroller Benchmark.");
}

void loop() {
    unsigned long now = millis();
    float deltaTime = (now - lastFrameTime) / 1000.0f;
    if (deltaTime <= 0.0f) deltaTime = 0.016f;
    lastFrameTime = now;

    // 1. Benchmark Physics & Animation Update
    myApp.update(deltaTime);

    // 2. Benchmark Multi-Band Render with DMA Double-Buffering + Dirty-Rect
    unsigned long renderStart = micros();
    myApp.render(gfx);
    unsigned long renderUs = micros() - renderStart;

    // 3. High-Precision FPS & Frame Timing Output
    frameCount++;
    if (now - lastFPSTime >= 1000) {
        float fps = frameCount * 1000.0f / (now - lastFPSTime);
        gfx.setFPS(fps);
        lastFPSTime = now;
        frameCount = 0;
        Serial.printf("[BENCHMARK METRICS] FPS: %.1f | Render: %lu us | Free Heap: %d bytes\n",
                      fps, renderUs, ESP.getFreeHeap());
    }
}
