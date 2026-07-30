#ifndef VIRTUAL_GAMEPAD_H
#define VIRTUAL_GAMEPAD_H

#include <Arduino.h>
#include "GFXContext.h"

/**
 * VirtualGamepad - On-Screen Touch Controller Subsystem for Retro Games
 * 
 * Renders on-screen D-Pad (Up, Down, Left, Right) and Action Buttons (A, B)
 * across the full 480x320 screen canvas.
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
     * Render the on-screen Touch D-Pad and Buttons onto full screen.
     */
    void render(GFXContext& gfx) {
        // D-Pad Cross Background
        uint16_t dpadColor = _state.left || _state.right || _state.up || _state.down ? 0xFFE0 : gfx.color565(40, 50, 70);
        gfx.fillRoundRectDirect(10, 235, 100, 30, 6, dpadColor);
        gfx.fillRoundRectDirect(45, 200, 30, 100, 6, dpadColor);
        gfx.drawRoundRectDirect(10, 235, 100, 30, 6, 0xFFFF);
        gfx.drawRoundRectDirect(45, 200, 30, 100, 6, 0xFFFF);

        // Action Button A
        uint16_t colorA = _state.btnA ? 0x07E0 : gfx.color565(0, 150, 0);
        gfx.drawCircleDirect(430, 260, 22, colorA, true);
        gfx.drawCircleDirect(430, 260, 22, 0xFFFF, false);
        gfx.drawTextDirect("A", 424, 253, 0xFFFF, 2, colorA);

        // Action Button B
        uint16_t colorB = _state.btnB ? 0xF800 : gfx.color565(150, 0, 0);
        gfx.drawCircleDirect(360, 260, 22, colorB, true);
        gfx.drawCircleDirect(360, 260, 22, 0xFFFF, false);
        gfx.drawTextDirect("B", 354, 253, 0xFFFF, 2, colorB);
    }
};

#endif // VIRTUAL_GAMEPAD_H
