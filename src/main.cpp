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

// Low-Level Driver Configuration
class LGFX_ILI9488 : public lgfx::LGFX_Device {
    lgfx::Panel_ILI9488 _panel_instance;
    lgfx::Bus_SPI       _bus_instance;
    lgfx::Light_PWM     _light_instance;

public:
    LGFX_ILI9488() {
        { // SPI Bus Configuration
            auto cfg = _bus_instance.config();
            cfg.spi_host   = VSPI_HOST;   // Use VSPI (SPI3) on ESP32
            cfg.spi_mode   = 0;
            cfg.freq_write = 27000000;    // 27 MHz SPI write speed
            cfg.freq_read  = 16000000;
            cfg.pin_sclk   = 18;          // SCK pin
            cfg.pin_mosi   = 23;          // MOSI pin
            cfg.pin_miso   = 19;          // MISO pin
            cfg.pin_dc     = 27;          // DC pin
            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }

        { // Panel Configuration
            auto cfg = _panel_instance.config();
            cfg.pin_cs           = 5;     // CS pin
            cfg.pin_rst          = 4;     // Reset pin
            cfg.pin_busy         = -1;
            cfg.panel_width      = 320;
            cfg.panel_height     = 480;
            cfg.offset_x         = 0;
            cfg.offset_y         = 0;
            cfg.offset_rotation  = 0;
            cfg.dummy_read_pixel = 8;
            cfg.readable         = true;
            cfg.invert           = false;
            cfg.rgb_order        = false;
            cfg.dlen_16bit       = false;
            cfg.bus_shared       = true;
            _panel_instance.config(cfg);
        }

        { // Backlight Configuration
            auto cfg = _light_instance.config();
            cfg.pin_bl      = 32;         // LED Backlight pin
            cfg.invert      = false;
            cfg.freq        = 44100;
            cfg.pwm_channel = 7;
            _light_instance.config(cfg);
            _panel_instance.setLight(&_light_instance);
        }

        setPanel(&_panel_instance);
    }
};

// Global Hardware Drivers
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
    pinMode(32, OUTPUT);
    digitalWrite(32, HIGH); // Backlight HIGH

    Serial.begin(115200);
    delay(300);
    Serial.println("\n[FRAMEWORK] Initializing Hardware Abstraction Framework...");

    tft.init();
    tft.setBrightness(255);
    tft.setRotation(1); // Landscape mode (480x320)
    tft.fillScreen(gfx.color565(8, 19, 19));

    spriteBuffer.setColorDepth(16);
    spriteBuffer.createSprite(SPRITE_SIZE, SPRITE_SIZE);

    // Initialize application using high-level drawing context
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
