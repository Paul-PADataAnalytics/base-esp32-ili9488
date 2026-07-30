#ifndef VIRTUAL_GAMEPAD_H
#define VIRTUAL_GAMEPAD_H

#include <Arduino.h>
#include "GFXContext.h"

/**
 * VirtualGamepad - On-Screen Touch Controller Subsystem for Retro Games
 * 
 * Scaled 320x160 touch gamepad layout.
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

        // D-Pad Touch Zone (Bottom Left: X 10-80, Y 90-155)
        if (tx >= 10 && tx <= 90 && ty >= 90 && ty <= 155) {
            int localX = tx - 50;
            int localY = ty - 120;

            if (abs(localX) > abs(localY)) {
                if (localX < 0) _state.left = true;
                else _state.right = true;
            } else {
                if (localY < 0) _state.up = true;
                else _state.down = true;
            }
        }

        // Action Button A Touch Zone (Bottom Right: X 260-305, Y 110-150)
        if (sqrt(pow(tx - 280, 2) + pow(ty - 130, 2)) <= 20) {
            _state.btnA = true;
        }

        // Action Button B Touch Zone (Bottom Right: X 215-255, Y 110-150)
        if (sqrt(pow(tx - 235, 2) + pow(ty - 130, 2)) <= 20) {
            _state.btnB = true;
        }
    }

    /**
     * Render the on-screen Touch D-Pad and Buttons onto 320x160 canvas.
     */
    void render(GFXContext& gfx) {
        // D-Pad Cross Background
        uint16_t dpadColor = _state.left || _state.right || _state.up || _state.down ? 0xFFE0 : gfx.color565(40, 50, 70);
        gfx.fillRoundRectDirect(10, 110, 80, 20, 4, dpadColor);
        gfx.fillRoundRectDirect(40, 90, 20, 60, 4, dpadColor);
        gfx.drawRoundRectDirect(10, 110, 80, 20, 4, 0xFFFF);
        gfx.drawRoundRectDirect(40, 90, 20, 60, 4, 0xFFFF);

        // Action Button A
        uint16_t colorA = _state.btnA ? 0x07E0 : gfx.color565(0, 150, 0);
        gfx.drawCircleDirect(280, 130, 14, colorA, true);
        gfx.drawCircleDirect(280, 130, 14, 0xFFFF, false);
        gfx.drawTextDirect("A", 276, 125, 0xFFFF, 1, colorA);

        // Action Button B
        uint16_t colorB = _state.btnB ? 0xF800 : gfx.color565(150, 0, 0);
        gfx.drawCircleDirect(235, 130, 14, colorB, true);
        gfx.drawCircleDirect(235, 130, 14, 0xFFFF, false);
        gfx.drawTextDirect("B", 231, 125, 0xFFFF, 1, colorB);
    }
};

#endif // VIRTUAL_GAMEPAD_H
