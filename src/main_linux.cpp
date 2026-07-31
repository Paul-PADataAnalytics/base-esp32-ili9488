#if defined(PLATFORM_LINUX) || !defined(ARDUINO)

#include "Arduino_Linux.h"
#include "hal/HALDisplay_Linux.h"
#include "GFXContext.h"
#include "LuaDemoApp.h"
#include <string>

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
    bool headless        = false;
    bool dumpUI          = false;
    bool injectTouch     = false;
    int  touchX          = 0;
    int  touchY          = 0;
    int  maxFrames       = -1; // -1 = run until window closed
    std::string screenshotPath = "";
    std::string evalLuaCode    = "";

    // Parse CLI arguments for AI Agent Inspection & Debugging
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--headless") {
            headless = true;
        } else if (arg == "--dump-ui") {
            dumpUI = true;
        } else if (arg == "--screenshot" && i + 1 < argc) {
            screenshotPath = argv[++i];
        } else if (arg == "--touch" && i + 2 < argc) {
            injectTouch = true;
            touchX = std::stoi(argv[++i]);
            touchY = std::stoi(argv[++i]);
        } else if (arg == "--eval" && i + 1 < argc) {
            evalLuaCode = argv[++i];
        } else if (arg == "--frames" && i + 1 < argc) {
            maxFrames = std::stoi(argv[++i]);
        }
    }

    if (headless || dumpUI || !screenshotPath.empty() || injectTouch || !evalLuaCode.empty()) {
        hal.setHeadless(true);
        if (maxFrames < 0) maxFrames = 10; // Default 10 frames for automated CLI runs
    }

    if (!hal.init()) {
        printf("[AGENT SIMULATOR ERROR] Failed to initialize HAL!\n");
        return 1;
    }

    gfx.setHALLinux(&hal);

    spriteBuffer0.setColorDepth(16);
    spriteBuffer0.createSprite(CANVAS_W, BAND_H);

    spriteBuffer1.setColorDepth(16);
    spriteBuffer1.createSprite(CANVAS_W, BAND_H);

    myApp.setup(gfx);

    // Inject touch press if requested
    if (injectTouch) {
        hal.injectTouchPress(touchX, touchY);
    }

    // Execute Live Lua code snippet if requested
    if (!evalLuaCode.empty()) {
        printf("[AGENT SIMULATOR] Executing Lua snippet: %s\n", evalLuaCode.c_str());
        myApp.getLuaEngine().executeString(evalLuaCode.c_str());
    }

    uint32_t lastFrameTime = millis();
    uint32_t lastFPSTime   = millis();
    int      frameCount    = 0;
    int      currentFrame  = 0;

    while (hal.isRunning()) {
        hal.pollEvents();

        uint32_t now = millis();
        float deltaTime = (now - lastFrameTime) / 1000.0f;
        if (deltaTime <= 0.0f) deltaTime = 0.016f;
        lastFrameTime = now;

        myApp.update(deltaTime, gfx);
        myApp.render(gfx);

        frameCount++;
        currentFrame++;

        if (now - lastFPSTime >= 1000) {
            float fps = frameCount * 1000.0f / (now - lastFPSTime);
            gfx.setFPS(fps);
            lastFPSTime = now;
            frameCount = 0;
        }

        // Release touch after 3 frames if injected
        if (injectTouch && currentFrame == 3) {
            hal.injectTouchRelease();
        }

        if (maxFrames > 0 && currentFrame >= maxFrames) {
            break;
        }

        if (!hal.isHeadless()) {
            delay(10); // Limit CPU usage in GUI mode (~100 FPS)
        }
    }

    // Dump UI JSON tree for AI Agents
    if (dumpUI) {
        printf("\n=== AGENT UI DUMP JSON ===\n");
        printf("%s\n", myApp.getUIManager().dumpUIJson().c_str());
        printf("==========================\n");
    }

    // Save Screenshot if requested
    if (!screenshotPath.empty()) {
        if (hal.savePPM(screenshotPath.c_str())) {
            printf("[AGENT SIMULATOR] Saved screenshot image to %s\n", screenshotPath.c_str());
        } else {
            printf("[AGENT SIMULATOR ERROR] Failed to save screenshot to %s\n", screenshotPath.c_str());
        }
    }

    return 0;
}

#endif
