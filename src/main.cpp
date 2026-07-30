/**
 * ESP32 Hardware-Abstracted Application Framework Main Entry
 */

#include <Arduino.h>
#include <LovyanGFX.hpp>
#include "GFXContext.h"
#include "AnimationApp.h"
#include "DisplayDriver.h"

// Instantiate Global Drivers
static LGFX_ILI9488 tft;
static LGFX_Sprite  spriteBuffer(&tft);

// 320x160 16-bit Double-Buffer Canvas (102.4 KB RAM - Guaranteed < 114.6 KB Max Alloc Heap!)
const int CANVAS_W = 320;
const int CANVAS_H = 160;

static GFXContext gfx(&tft, &spriteBuffer, 0, 0, CANVAS_W, CANVAS_H);
static AnimationApp myApp;

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
    Serial.println("\n[FRAMEWORK] Initializing Display & Touch HAL Engine...");
    Serial.printf("[FRAMEWORK MEMORY] Free Heap: %d bytes, Max Alloc: %d bytes\n", ESP.getFreeHeap(), ESP.getMaxAllocHeap());

    // 2. Initialize Display Driver
    tft.init();
    tft.setRotation(1); // Landscape mode (480x320)
    tft.fillScreen(tft.color565(6, 14, 25));

    // Allocate 320x160 16-bit RGB565 double-buffer sprite (102.4 KB - guaranteed fit under 114.6 KB max heap)
    spriteBuffer.setColorDepth(16);
    void* ptr = spriteBuffer.createSprite(CANVAS_W, CANVAS_H);
    if (ptr) {
        spriteBuffer.setPivot(CANVAS_W / 2, CANVAS_H / 2); // Center pivot for full-screen hardware scaling
        Serial.println("[FRAMEWORK] SUCCESS: Allocated 16-bit RGB565 zero-flicker double-buffer canvas.");
    } else {
        Serial.println("[FRAMEWORK ERROR] Memory allocation failed for sprite buffer!");
    }

    // 3. Initialize application using high-level drawing context
    myApp.setup(gfx);

    lastFrameTime = millis();
    lastFPSTime   = millis();
    Serial.println("[FRAMEWORK] Setup Complete. Running Application.");
}

void loop() {
    unsigned long now = millis();
    float deltaTime = (now - lastFrameTime) / 1000.0f;
    if (deltaTime <= 0.0f) deltaTime = 0.016f;
    lastFrameTime = now;

    // 1. Application Physics/State Update
    myApp.update(deltaTime);

    // 2. Application Render Pass (Atomic DMA Double-Buffer Pass)
    tft.startWrite();
    myApp.render(gfx);
    tft.endWrite();

    // 3. FPS Calculation
    frameCount++;
    if (now - lastFPSTime >= 1000) {
        float fps = frameCount * 1000.0f / (now - lastFPSTime);
        gfx.setFPS(fps);
        lastFPSTime = now;
        frameCount = 0;
        Serial.printf("[FRAMEWORK ENGINE] Running - FPS: %.1f\n", fps);
    }
}
