#ifndef VIRTUAL_GAMEPAD_H
#define VIRTUAL_GAMEPAD_H

#include <Arduino.h>
#include <LovyanGFX.hpp>
#include "GFXContext.h"

/**
 * VirtualGamepad - On-Screen Touch Controller Subsystem for Retro Games
 * 
 * Provides an on-screen D-Pad (Up, Down, Left, Right) and Action Buttons (A, B).
 * Renders cleanly into the double-buffered sprite to guarantee 0% flicker.
 */
class VirtualGamepad {
public:
    struct ButtonState {
        bool up;
        bool down;
        bool left;
        bool right;
        bool btnA;
        bool btnB;
    };

private:
    ButtonState _state;

public:
    VirtualGamepad() {
        memset(&_state, 0, sizeof(ButtonState));
    }

    const ButtonState& getState() const { return _state; }

    /**
     * Process touch input coordinates and update D-Pad & Button states.
     */
    void update(GFXContext& gfx) {
        memset(&_state, 0, sizeof(ButtonState));

        int tx, ty;
        if (!gfx.getTouch(&tx, &ty)) return;

        // D-Pad Touch Zone (Bottom Left: X 10-110, Y 200-300)
        if (tx >= 10 && tx <= 110 && ty >= 200 && ty <= 300) {
            int localX = tx - 60;
            int localY = ty - 250;

            if (abs(localX) > abs(localY)) {
                if (localX < 0) _state.left = true;
                else _state.right = true;
            } else {
                if (localY < 0) _state.up = true;
                else _state.down = true;
            }
        }

        // Action Button A Touch Zone (Bottom Right: X 400-460, Y 230-290)
        if (sqrt(pow(tx - 430, 2) + pow(ty - 260, 2)) <= 25) {
            _state.btnA = true;
        }

        // Action Button B Touch Zone (Bottom Right: X 330-390, Y 230-290)
        if (sqrt(pow(tx - 360, 2) + pow(ty - 260, 2)) <= 25) {
            _state.btnB = true;
        }
    }

    /**
     * Render the on-screen Touch D-Pad and Buttons INTO the sprite buffer (0% Flicker).
     */
    void render(LGFX_Sprite* buffer, uint16_t primaryColor = 0x07FF) {
        if (!buffer) return;

        // D-Pad Cross Background
        uint16_t dpadColor = _state.left || _state.right || _state.up || _state.down ? 0xFFE0 : 0x2A9D;
        buffer->fillRoundRect(10, 235, 100, 30, 6, dpadColor);
        buffer->fillRoundRect(45, 200, 30, 100, 6, dpadColor);
        buffer->drawRoundRect(10, 235, 100, 30, 6, 0xFFFF);
        buffer->drawRoundRect(45, 200, 30, 100, 6, 0xFFFF);

        // Action Button A
        uint16_t colorA = _state.btnA ? 0x07E0 : 0x0400;
        buffer->fillCircle(430, 260, 22, colorA);
        buffer->drawCircle(430, 260, 22, 0xFFFF);
        buffer->setTextColor(0xFFFF, colorA);
        buffer->setTextSize(2);
        buffer->setCursor(424, 253);
        buffer->print("A");

        // Action Button B
        uint16_t colorB = _state.btnB ? 0xF800 : 0x8000;
        buffer->fillCircle(360, 260, 22, colorB);
        buffer->drawCircle(360, 260, 22, 0xFFFF);
        buffer->setTextColor(0xFFFF, colorB);
        buffer->setTextSize(2);
        buffer->setCursor(354, 253);
        buffer->print("B");
    }
};

#endif // VIRTUAL_GAMEPAD_H
