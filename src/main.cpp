/**
 * ESP32 Hardware-Abstracted Application Framework Main Entry
 * 
 * Hardware control is isolated inside this entry point and GFXContext.
 * User application code (AnimationApp) contains ZERO hardware registers or SPI logic.
 */

#include <Arduino.h>
#include <LovyanGFX.hpp>
#include "GFXContext.h"
#include "AnimationApp.h"
#include "DisplayDriver.h"

// Instantiate Global Drivers
static LGFX_ILI9488 tft;
static LGFX_Sprite  spriteBuffer(&tft);

// Application & Abstraction Layer
const int SPRITE_SIZE = 180;
const int SPRITE_X    = (480 - SPRITE_SIZE) / 2;
const int SPRITE_Y    = (320 - SPRITE_SIZE) / 2 + 10;

static GFXContext gfx(&tft, &spriteBuffer, SPRITE_X, SPRITE_Y, SPRITE_SIZE, SPRITE_SIZE);
static AnimationApp myApp;

unsigned long lastFrameTime = 0;
unsigned long lastFPSTime   = 0;
int           frameCount    = 0;

void setup() {
    // 1. Deselect Touch CS (GPIO 33) & Turn On Backlight (GPIO 32)
    pinMode(33, OUTPUT);
    digitalWrite(33, HIGH); // Pull TCS HIGH (deselected) so LCD SPI init is 100% clean

    pinMode(32, OUTPUT);
    digitalWrite(32, HIGH); // Backlight HIGH

    Serial.begin(115200);
    delay(300);
    Serial.println("\n[FRAMEWORK] Initializing Display & Touch HAL Engine...");

    // 2. Initialize Display Driver
    tft.init();
    tft.setBrightness(255);
    tft.setRotation(1); // Landscape mode (480x320)
    tft.fillScreen(gfx.color565(8, 19, 19));

    spriteBuffer.setColorDepth(16);
    spriteBuffer.createSprite(SPRITE_SIZE, SPRITE_SIZE);

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
