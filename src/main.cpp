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
static LGFX_Sprite  spriteBuffer(&tft);

// Native 480x320 Viewport & 480x80 Band Buffer (76.8 KB RAM)
const int CANVAS_W = 480;
const int CANVAS_H = 320;
const int BAND_H   = 80;

static GFXContext gfx(&tft, &spriteBuffer, 0, 0, CANVAS_W, CANVAS_H);
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

    // Diagnostic Color Test on LCD Panel
    tft.fillRect(20, 20, 40, 40, tft.color565(255, 0, 0));   // RED
    tft.fillRect(70, 20, 40, 40, tft.color565(0, 255, 0));   // GREEN
    tft.fillRect(120, 20, 40, 40, tft.color565(0, 0, 255));  // BLUE
    tft.fillRect(170, 20, 40, 40, tft.color565(139, 69, 19)); // BROWN

    // Direct pushImage of Tile 0 (Grass Top) and Tile 1 (Deep Dirt)
    uint16_t testBuf[32 * 32];
    for (int i = 0; i < 32 * 32; i++) {
        testBuf[i] = pgm_read_word(&DIRT_TILESET_32x32[i]);
    }
    tft.pushImage(220, 20, 32, 32, testBuf); // Direct push without swap
    
    for (int i = 0; i < 32 * 32; i++) {
        uint16_t p = pgm_read_word(&DIRT_TILESET_32x32[i]);
        testBuf[i] = (p >> 8) | (p << 8);
    }
    tft.pushImage(260, 20, 32, 32, testBuf); // Direct push WITH swap

    // Allocate 480x80 16-bit RGB565 band buffer (76.8 KB RAM)
    spriteBuffer.setColorDepth(16);
    void* ptr = spriteBuffer.createSprite(CANVAS_W, BAND_H);
    if (ptr) {
        Serial.println("[BENCHMARK] SUCCESS: Allocated 480x80 16-bit RGB565 band buffer (76.8 KB).");
    } else {
        Serial.println("[BENCHMARK ERROR] Band buffer allocation failed!");
    }

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

    // 2. Benchmark Multi-Band 0% Flicker Render Pass
    tft.startWrite();
    myApp.render(gfx);
    tft.endWrite();

    // 3. High-Precision FPS & Frame Timing Output
    frameCount++;
    if (now - lastFPSTime >= 1000) {
        float fps = frameCount * 1000.0f / (now - lastFPSTime);
        gfx.setFPS(fps);
        lastFPSTime = now;
        frameCount = 0;
        Serial.printf("[BENCHMARK ENGINE] Running - FPS: %.1f, Free Heap: %d\n", fps, ESP.getFreeHeap());
    }
}
