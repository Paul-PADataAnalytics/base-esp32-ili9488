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

// Full 480x320 Display Canvas
const int CANVAS_W = 480;
const int CANVAS_H = 320;
const int CANVAS_X = 0;
const int CANVAS_Y = 0;

static GFXContext gfx(&tft, &spriteBuffer, CANVAS_X, CANVAS_Y, CANVAS_W, CANVAS_H);
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

    // 2. Initialize Display Driver
    tft.init();
    tft.setRotation(1); // Landscape mode (480x320)
    tft.fillScreen(gfx.color565(8, 19, 19));

    // Initialize 8-bit double buffer canvas (153.6 KB RAM) for 100% zero-tear DMA flushing
    spriteBuffer.setColorDepth(8);
    void* ptr = spriteBuffer.createSprite(CANVAS_W, CANVAS_H);
    if (!ptr) {
        Serial.println("[FRAMEWORK ERROR] Failed to allocate full 480x320 double-buffer canvas!");
    } else {
        Serial.println("[FRAMEWORK] Allocated full 480x320 zero-flicker double-buffer canvas.");
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

    // 2. Application Render Pass
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
