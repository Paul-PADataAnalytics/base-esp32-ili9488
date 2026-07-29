#ifndef VIRTUAL_GAMEPAD_H
#define VIRTUAL_GAMEPAD_H

#include <Arduino.h>
#include <LovyanGFX.hpp>
#include "GFXContext.h"

/**
 * VirtualGamepad - On-Screen Touch Controller Subsystem for Retro Games
 * 
 * Provides an on-screen D-Pad (Up, Down, Left, Right) and Action Buttons (A, B).
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
     * Render the on-screen Touch D-Pad and Buttons onto the UI Overlay layer.
     */
    void render(GFXContext& gfx) {
        // D-Pad Cross Background
        gfx.fillRoundRectDirect(10, 235, 100, 30, 6, _state.left || _state.right ? 0xFFE0 : gfx.color565(40, 50, 70));
        gfx.fillRoundRectDirect(45, 200, 30, 100, 6, _state.up || _state.down ? 0xFFE0 : gfx.color565(40, 50, 70));

        // Action Button A
        uint16_t colorA = _state.btnA ? 0x07E0 : gfx.color565(0, 180, 0);
        gfx.drawCircleDirect(430, 260, 22, colorA, true);
        gfx.drawTextDirect("A", 423, 252, 0xFFFF, 2);

        // Action Button B
        uint16_t colorB = _state.btnB ? 0xF800 : gfx.color565(180, 0, 0);
        gfx.drawCircleDirect(360, 260, 22, colorB, true);
        gfx.drawTextDirect("B", 353, 252, 0xFFFF, 2);
    }
};

#endif // VIRTUAL_GAMEPAD_H
