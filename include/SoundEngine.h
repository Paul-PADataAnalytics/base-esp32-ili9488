#ifndef SOUND_ENGINE_H
#define SOUND_ENGINE_H

#include <Arduino.h>

/**
 * SoundEngine - ESP32 8-Bit Tone & SFX Synthesizer Engine
 * 
 * Drives an optional passive piezo buzzer or audio pin (GPIO 25 or 26) using
 * ESP32 LEDC PWM. Includes pre-programmed retro sound effects.
 */
class SoundEngine {
private:
    int  _audioPin;
    bool _enabled;

public:
    SoundEngine(int audioPin = 25) : _audioPin(audioPin), _enabled(true) {}

    void init() {
        if (_audioPin >= 0) {
            pinMode(_audioPin, OUTPUT);
            digitalWrite(_audioPin, LOW);
        }
    }

    void setEnabled(bool enabled) { _enabled = enabled; }
    bool isEnabled() const { return _enabled; }

    void tone(unsigned int frequency, unsigned long duration = 0) {
        if (!_enabled || _audioPin < 0) return;
        #if defined(ESP32)
            ledcAttachPin(_audioPin, 0);
            ledcWriteTone(0, frequency);
            if (duration > 0) {
                delay(duration);
                ledcDetachPin(_audioPin);
            }
        #endif
    }

    void noTone() {
        if (_audioPin < 0) return;
        #if defined(ESP32)
            ledcDetachPin(_audioPin);
            digitalWrite(_audioPin, LOW);
        #endif
    }

    // --- Pre-programmed Retro 8-Bit SFX ---

    void playCoin() {
        if (!_enabled) return;
        tone(988, 80);  // B5
        tone(1319, 120); // E6
        noTone();
    }

    void playJump() {
        if (!_enabled) return;
        for (int freq = 150; freq < 600; freq += 40) {
            tone(freq, 8);
        }
        noTone();
    }

    void playExplosion() {
        if (!_enabled) return;
        for (int i = 0; i < 15; i++) {
            tone(random(40, 200), random(10, 25));
        }
        noTone();
    }

    void playLaser() {
        if (!_enabled) return;
        for (int freq = 800; freq > 200; freq -= 50) {
            tone(freq, 5);
        }
        noTone();
    }

    void playPowerup() {
        if (!_enabled) return;
        tone(330, 70); // E4
        tone(392, 70); // G4
        tone(659, 70); // E5
        tone(523, 70); // C5
        tone(587, 70); // D5
        tone(784, 140); // G5
        noTone();
    }
};

#endif // SOUND_ENGINE_H
