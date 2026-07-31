#if defined(PLATFORM_LINUX) || !defined(ARDUINO)

#include "Arduino_Linux.h"
#include "hal/HALDisplay_Linux.h"
#include "GFXContext.h"
#include "LuaDemoApp.h"

// Instantiate Native Linux SDL2 HAL
static HALDisplay_Linux hal(480, 320, 2); // 480x320 canvas, 2x window scale (960x640)

// Band buffers
static LGFX_Sprite spriteBuffer0;
static LGFX_Sprite spriteBuffer1;

const int CANVAS_W = 480;
const int CANVAS_H = 320;
const int BAND_H   = 80;

static GFXContext gfx(nullptr, &spriteBuffer0, &spriteBuffer1, 0, 0, CANVAS_W, CANVAS_H);
static LuaDemoApp myApp;

int main(int argc, char* argv[]) {
    printf("[NATIVE DESKTOP SIMULATOR] Starting ESP32 ILI9488 Native Linux Simulator...\n");

    if (!hal.init()) {
        printf("[NATIVE DESKTOP ERROR] Failed to initialize SDL2 HAL!\n");
        return 1;
    }

    gfx.setHALLinux(&hal);

    spriteBuffer0.setColorDepth(16);
    spriteBuffer0.createSprite(CANVAS_W, BAND_H);

    spriteBuffer1.setColorDepth(16);
    spriteBuffer1.createSprite(CANVAS_W, BAND_H);

    myApp.setup(gfx);

    uint32_t lastFrameTime = millis();
    uint32_t lastFPSTime   = millis();
    int      frameCount    = 0;

    while (hal.isRunning()) {
        hal.pollEvents(); // Process mouse clicks, drags, window close, etc.

        uint32_t now = millis();
        float deltaTime = (now - lastFrameTime) / 1000.0f;
        if (deltaTime <= 0.0f) deltaTime = 0.016f;
        lastFrameTime = now;

        myApp.update(deltaTime, gfx);
        myApp.render(gfx);

        frameCount++;
        if (now - lastFPSTime >= 1000) {
            float fps = frameCount * 1000.0f / (now - lastFPSTime);
            gfx.setFPS(fps);
            lastFPSTime = now;
            frameCount = 0;
            printf("[DESKTOP METRICS] FPS: %.1f | Free Heap: %u bytes\n", fps, ESP.getFreeHeap());
        }

        delay(10); // Limit CPU usage (~100 FPS cap)
    }

    printf("[NATIVE DESKTOP SIMULATOR] Exited cleanly.\n");
    return 0;
}

#endif
