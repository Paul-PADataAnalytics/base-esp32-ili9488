#ifndef SAVE_SYSTEM_H
#define SAVE_SYSTEM_H

#include <Arduino.h>
#include <Preferences.h>

/**
 * SaveSystem - ESP32 Non-Volatile Storage (NVS) Persistence Engine
 * 
 * Safely saves game progress, high scores, settings, and player data across reboots.
 */
class SaveSystem {
private:
    Preferences _prefs;
    String      _namespaceName;

public:
    SaveSystem(const char* nsName = "hal_game") : _namespaceName(nsName) {}

    void begin() {
        _prefs.begin(_namespaceName.c_str(), false);
    }

    void end() {
        _prefs.end();
    }

    // High Score Storage
    void saveHighScore(int score) {
        _prefs.begin(_namespaceName.c_str(), false);
        int currentHighScore = _prefs.getInt("highscore", 0);
        if (score > currentHighScore) {
            _prefs.putInt("highscore", score);
            Serial.printf("[SAVE SYSTEM] New High Score Saved: %d\n", score);
        }
        _prefs.end();
    }

    int getHighScore() {
        _prefs.begin(_namespaceName.c_str(), true); // Read-only
        int score = _prefs.getInt("highscore", 0);
        _prefs.end();
        return score;
    }

    // Generic Integer Key/Value Storage
    void saveInt(const char* key, int value) {
        _prefs.begin(_namespaceName.c_str(), false);
        _prefs.putInt(key, value);
        _prefs.end();
    }

    int getInt(const char* key, int defaultValue = 0) {
        _prefs.begin(_namespaceName.c_str(), true);
        int val = _prefs.getInt(key, defaultValue);
        _prefs.end();
        return val;
    }

    // Generic String Storage
    void saveString(const char* key, const char* value) {
        _prefs.begin(_namespaceName.c_str(), false);
        _prefs.putString(key, value);
        _prefs.end();
    }

    String getString(const char* key, const char* defaultValue = "") {
        _prefs.begin(_namespaceName.c_str(), true);
        String val = _prefs.getString(key, defaultValue);
        _prefs.end();
        return val;
    }

    void clearAll() {
        _prefs.begin(_namespaceName.c_str(), false);
        _prefs.clear();
        _prefs.end();
        Serial.println("[SAVE SYSTEM] All NVS save data cleared.");
    }
};

#endif // SAVE_SYSTEM_H
