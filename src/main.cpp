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

// Native 480x320 Display Viewport
const int CANVAS_W = 480;
const int CANVAS_H = 320;

static GFXContext gfx(&tft, nullptr, 0, 0, CANVAS_W, CANVAS_H);
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
    tft.setRotation(1); // Landscape mode (480x320 native resolution)
    tft.fillScreen(tft.color565(6, 14, 25));

    // 3. Initialize application using high-level drawing context
    myApp.setup(gfx);

    lastFrameTime = millis();
    lastFPSTime   = millis();
    Serial.println("[FRAMEWORK] Setup Complete. Running Application at Native 480x320 Resolution.");
}

void loop() {
    unsigned long now = millis();
    float deltaTime = (now - lastFrameTime) / 1000.0f;
    if (deltaTime <= 0.0f) deltaTime = 0.016f;
    lastFrameTime = now;

    // 1. Application Physics/State Update
    myApp.update(deltaTime);

    // 2. High-Speed Hardware SPI DMA Batch Render Transaction
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
