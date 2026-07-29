#ifndef ANIMATION_APP_H
#define ANIMATION_APP_H

#include "BaseApp.h"
#include "GFXContext.h"
#include <Arduino.h>

/**
 * 3D Rotating Cube & Bouncing Spheres Demo Application
 * 
 * Contains ZERO hardware registers or SPI driver code.
 * Implements BaseApp lifecycle interface (setup, update, render).
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
    AnimationApp() : _angleX(0), _angleY(0), _angleZ(0) {}

    void setup(GFXContext& gfx) override {
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
    }

    void update(float deltaTime) override {
        _angleX += 140.0f * deltaTime;
        _angleY += 180.0f * deltaTime;
        _angleZ += 90.0f * deltaTime;

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

        // 1. Erase & Draw Spheres outside sprite area
        for (int i = 0; i < 4; i++) {
            if (!gfx.overlapsBuffer(_balls[i].oldX, _balls[i].oldY, _balls[i].radius)) {
                gfx.eraseCircleDirect((int)_balls[i].oldX, (int)_balls[i].oldY, (int)_balls[i].radius, spaceColor);
            }

            if (!gfx.overlapsBuffer(_balls[i].x, _balls[i].y, _balls[i].radius)) {
                gfx.drawCircleDirect((int)_balls[i].x, (int)_balls[i].y, (int)_balls[i].radius, _balls[i].color, true);
                gfx.drawCircleDirect((int)_balls[i].x, (int)_balls[i].y, (int)_balls[i].radius + 1, 0xFFFF, false);
            }
        }

        // 2. Render 3D Cube & Overlapping Spheres inside Off-Screen Sprite Buffer
        gfx.clearBuffer(spaceColor);

        int spriteSize = gfx.getSpriteW();
        int projX[8], projY[8];
        for (int i = 0; i < 8; i++) {
            project(_cubeVertices[i], projX[i], projY[i], spriteSize);
        }

        for (int i = 0; i < 12; i++) {
            int p1 = _cubeEdges[i][0];
            int p2 = _cubeEdges[i][1];
            uint16_t wireColor = gfx.color565(
                (uint8_t)(128 + 127 * sin(_angleX * DEG_TO_RAD)),
                (uint8_t)(128 + 127 * sin(_angleY * DEG_TO_RAD + 2.0)),
                255
            );
            gfx.drawLineBuffer(projX[p1], projY[p1], projX[p2], projY[p2], wireColor);
            gfx.drawCircleBuffer(projX[p1], projY[p1], 3, 0xFFE0, true);
        }

        for (int i = 0; i < 4; i++) {
            if (gfx.overlapsBuffer(_balls[i].x, _balls[i].y, _balls[i].radius)) {
                int localX = (int)(_balls[i].x - gfx.getSpriteX());
                int localY = (int)(_balls[i].y - gfx.getSpriteY());
                int r      = (int)_balls[i].radius;

                gfx.drawCircleBuffer(localX, localY, r, _balls[i].color, true);
                gfx.drawCircleBuffer(localX, localY, r + 1, 0xFFFF, false);
            }
        }

        // Push off-screen sprite buffer to LCD via SPI DMA
        gfx.pushBuffer();

        // 3. UI Header Text & FPS
        gfx.drawTextDirect("ESP32 Framework App", 15, 10, gfx.color565(0, 255, 255), 2);
        gfx.drawRectDirect(gfx.getSpriteX() - 1, gfx.getSpriteY() - 1, gfx.getSpriteW() + 2, gfx.getSpriteH() + 2, 0xFFFF);

        char fpsBuf[16];
        snprintf(fpsBuf, sizeof(fpsBuf), "FPS: %.1f ", gfx.getFPS());
        gfx.drawTextDirect(fpsBuf, 340, 10, gfx.color565(0, 255, 120), 2);
    }
};

#endif // ANIMATION_APP_H
