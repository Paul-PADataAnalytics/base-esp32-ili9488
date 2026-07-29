#ifndef ANIMATION_APP_H
#define ANIMATION_APP_H

#include "BaseApp.h"
#include "GFXContext.h"
#include "SleepManager.h"
#include "SampleIcons.h"
#include <Arduino.h>

// RTC memory variable that survives Deep Sleep reboots!
RTC_DATA_ATTR static int rtcWakeupCount = 0;

/**
 * Interactive 3D Cube & Bouncing Spheres Demo with Sleep Mode Choice & Image Loading
 */
class AnimationApp : public BaseApp {

private:
    struct Point3D {
        float x, y, z;
    };

    struct Ball {
        float x, y;
        float vx, vy;
        float radius;
        uint16_t color;
        float oldX, oldY;
    };

    Point3D _cubeVertices[8] = {
        {-42, -42, -42}, { 42, -42, -42}, { 42,  42, -42}, {-42,  42, -42},
        {-42, -42,  42}, { 42, -42,  42}, { 42,  42,  42}, {-42,  42,  42}
    };

    int _cubeEdges[12][2] = {
        {0, 1}, {1, 2}, {2, 3}, {3, 0},
        {4, 5}, {5, 6}, {6, 7}, {7, 4},
        {0, 4}, {1, 5}, {2, 6}, {3, 7}
    };

    Ball _balls[4];
    float _angleX, _angleY, _angleZ;

    bool _touched;
    int  _touchX, _touchY;
    int  _lastTouchX, _lastTouchY;

    SleepManager _sleepManager;

    void project(Point3D p, int &screenX, int &screenY, int spriteSize) {
        float radX = _angleX * DEG_TO_RAD;
        float y1 = p.y * cos(radX) - p.z * sin(radX);
        float z1 = p.y * sin(radX) + p.z * cos(radX);

        float radY = _angleY * DEG_TO_RAD;
        float x2 = p.x * cos(radY) + z1 * sin(radY);
        float z2 = -p.x * sin(radY) + z1 * cos(radY);

        float radZ = _angleZ * DEG_TO_RAD;
        float x3 = x2 * cos(radZ) - y1 * sin(radZ);
        float y3 = x2 * sin(radZ) + y1 * cos(radZ);

        float distance = 170.0f;
        float fov = distance / (distance + z2);

        screenX = (int)(spriteSize / 2 + x3 * fov);
        screenY = (int)(spriteSize / 2 + y3 * fov);
    }

public:
    AnimationApp() 
        : _angleX(0), _angleY(0), _angleZ(0),
          _touched(false), _touchX(0), _touchY(0),
          _lastTouchX(0), _lastTouchY(0),
          _sleepManager(15, SleepMode::DEEP_SLEEP) {} // Choice: DEEP_SLEEP after 15s inactivity

    void setup(GFXContext& gfx) override {
        // Register Developer Pre-Sleep & Post-Wake Callbacks
        _sleepManager.onPreSleep([this]() {
            Serial.println("[APP CALLBACK] Application saving state & pausing before sleep...");
        });

        _sleepManager.onPostWake([this]() {
            rtcWakeupCount++;
            Serial.printf("[APP CALLBACK] Application restored state after wake! Total Wakeups: %d\n", rtcWakeupCount);
        });

        // Assistant check for Deep Sleep reboot wakeup
        _sleepManager.checkAndNotifyDeepSleepWakeup();

        uint16_t colors[4] = {
            gfx.color565(0, 255, 255),
            gfx.color565(255, 255, 0),
            gfx.color565(255, 0, 255),
            gfx.color565(0, 255, 120)
        };
        for (int i = 0; i < 4; i++) {
            _balls[i].x = random(30, gfx.getWidth() - 30);
            _balls[i].y = random(50, gfx.getHeight() - 30);
            _balls[i].vx = random(3, 6) * (random(0, 2) ? 1 : -1);
            _balls[i].vy = random(3, 6) * (random(0, 2) ? 1 : -1);
            _balls[i].radius = random(12, 16);
            _balls[i].color = colors[i];
            _balls[i].oldX = _balls[i].x;
            _balls[i].oldY = _balls[i].y;
        }

        _sleepManager.resetInactivityTimer();
    }

    void update(float deltaTime) override {
        if (!_touched) {
            _angleX += 140.0f * deltaTime;
            _angleY += 180.0f * deltaTime;
            _angleZ += 90.0f * deltaTime;
        }

        for (int i = 0; i < 4; i++) {
            _balls[i].oldX = _balls[i].x;
            _balls[i].oldY = _balls[i].y;

            _balls[i].x += _balls[i].vx;
            _balls[i].y += _balls[i].vy;

            if (_balls[i].x - _balls[i].radius <= 5 || _balls[i].x + _balls[i].radius >= 475) {
                _balls[i].vx *= -1;
            }
            if (_balls[i].y - _balls[i].radius <= 35 || _balls[i].y + _balls[i].radius >= 315) {
                _balls[i].vy *= -1;
            }
        }
    }

    void render(GFXContext& gfx) override {
        uint16_t spaceColor = gfx.color565(8, 19, 19);

        // Touch Input & Inactivity Reset
        _touched = gfx.getTouch(&_touchX, &_touchY);
        if (_touched) {
            _sleepManager.resetInactivityTimer();

            if (_lastTouchX > 0 && _lastTouchY > 0) {
                _angleY += (_touchX - _lastTouchX) * 1.5f;
                _angleX += (_touchY - _lastTouchY) * 1.5f;
            }
            _lastTouchX = _touchX;
            _lastTouchY = _touchY;

            _balls[0].x = _touchX;
            _balls[0].y = _touchY;
        } else {
            _lastTouchX = 0;
            _lastTouchY = 0;
        }

        // Check Auto-Sleep Trigger
        if (_sleepManager.shouldAutoSleep()) {
            _sleepManager.triggerSleep(gfx);
            return;
        }

        // 1. Erase & Draw Spheres
        for (int i = 0; i < 4; i++) {
            if (!gfx.overlapsBuffer(_balls[i].oldX, _balls[i].oldY, _balls[i].radius)) {
                gfx.eraseCircleDirect((int)_balls[i].oldX, (int)_balls[i].oldY, (int)_balls[i].radius, spaceColor);
            }

            if (!gfx.overlapsBuffer(_balls[i].x, _balls[i].y, _balls[i].radius)) {
                gfx.drawCircleDirect((int)_balls[i].x, (int)_balls[i].y, (int)_balls[i].radius, _balls[i].color, true);
                gfx.drawCircleDirect((int)_balls[i].x, (int)_balls[i].y, (int)_balls[i].radius + 1, 0xFFFF, false);
            }
        }

        // 2. Render 3D Cube & Image Bitmap Asset inside Sprite Buffer
        gfx.clearBuffer(spaceColor);

        int spriteSize = gfx.getSpriteW();
        int projX[8], projY[8];
        for (int i = 0; i < 8; i++) {
            project(_cubeVertices[i], projX[i], projY[i], spriteSize);
        }

        for (int i = 0; i < 12; i++) {
            int p1 = _cubeEdges[i][0];
            int p2 = _cubeEdges[i][1];
            uint16_t wireColor = _touched ? gfx.color565(255, 120, 0) : gfx.color565(
                (uint8_t)(128 + 127 * sin(_angleX * DEG_TO_RAD)),
                (uint8_t)(128 + 127 * sin(_angleY * DEG_TO_RAD + 2.0)),
                255
            );
            gfx.drawLineBuffer(projX[p1], projY[p1], projX[p2], projY[p2], wireColor);
            gfx.drawCircleBuffer(projX[p1], projY[p1], 3, 0xFFE0, true);
        }

        // Render transparent 16x16 bitmap icon at center of cube sprite!
        gfx.pushImageTransparent(spriteSize / 2 - 8, spriteSize / 2 - 8, 16, 16, ICON_POWER_16x16, 0x0000);

        for (int i = 0; i < 4; i++) {
            if (gfx.overlapsBuffer(_balls[i].x, _balls[i].y, _balls[i].radius)) {
                int localX = (int)(_balls[i].x - gfx.getSpriteX());
                int localY = (int)(_balls[i].y - gfx.getSpriteY());
                int r      = (int)_balls[i].radius;

                gfx.drawCircleBuffer(localX, localY, r, _balls[i].color, true);
                gfx.drawCircleBuffer(localX, localY, r + 1, 0xFFFF, false);
            }
        }

        gfx.pushBuffer();

        // 3. UI Header Text, Gradient & Sleep Mode Info
        gfx.drawGradientRectDirect(0, 0, 480, 32, gfx.color565(15, 30, 60), gfx.color565(8, 19, 19), true);
        gfx.drawTextDirect("HAL Framework v1.2", 15, 8, gfx.color565(0, 255, 255), 2);
        gfx.drawRectDirect(gfx.getSpriteX() - 1, gfx.getSpriteY() - 1, gfx.getSpriteW() + 2, gfx.getSpriteH() + 2, 0xFFFF);

        uint32_t secRemaining = _sleepManager.getInactivitySecondsRemaining();
        char sleepBuf[32];
        snprintf(sleepBuf, sizeof(sleepBuf), "DeepSleep: %lds ", (long)secRemaining);
        gfx.drawTextDirect(sleepBuf, 290, 8, gfx.color565(255, 165, 0), 2);

        if (rtcWakeupCount > 0) {
            char wakeBuf[32];
            snprintf(wakeBuf, sizeof(wakeBuf), "Wakes: %d", rtcWakeupCount);
            gfx.drawTextDirect(wakeBuf, 15, 295, gfx.color565(0, 255, 120), 2);
        }
    }
};

#endif // ANIMATION_APP_H
