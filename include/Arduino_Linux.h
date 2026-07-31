#ifndef ARDUINO_LINUX_H
#define ARDUINO_LINUX_H

#if defined(PLATFORM_LINUX) || !defined(ARDUINO)

#include <iostream>
#include <chrono>
#include <thread>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <string>
#include <algorithm>

// --- Timing Stubs ---
inline uint32_t millis() {
    using namespace std::chrono;
    static auto start = steady_clock::now();
    return static_cast<uint32_t>(duration_cast<milliseconds>(steady_clock::now() - start).count());
}

inline uint64_t micros() {
    using namespace std::chrono;
    static auto start = steady_clock::now();
    return static_cast<uint64_t>(duration_cast<microseconds>(steady_clock::now() - start).count());
}

inline void delay(uint32_t ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

inline void delayMicroseconds(uint32_t us) {
    std::this_thread::sleep_for(std::chrono::microseconds(us));
}

// --- Serial Stub ---
class SerialMock {
public:
    void begin(unsigned long baud) {}
    void print(const char* str) { std::cout << str; }
    void print(int val) { std::cout << val; }
    void print(float val) { std::cout << val; }
    void println(const char* str = "") { std::cout << str << std::endl; }
    void println(int val) { std::cout << val << std::endl; }
    void println(float val) { std::cout << val << std::endl; }

    template<typename... Args>
    void printf(const char* fmt, Args... args) {
        ::printf(fmt, args...);
    }
};

static SerialMock Serial;

// --- GPIO & Hardware Stubs ---
#define INPUT 0x0
#define OUTPUT 0x1
#define HIGH 0x1
#define LOW 0x0

inline void pinMode(uint8_t pin, uint8_t mode) {}
inline void digitalWrite(uint8_t pin, uint8_t val) {}
inline int digitalRead(uint8_t pin) { return LOW; }

// --- PROGMEM Stubs ---
#ifndef PROGMEM
#define PROGMEM
#endif

#ifndef pgm_read_word
#define pgm_read_word(addr) (*(const uint16_t*)(addr))
#endif

#ifndef pgm_read_byte
#define pgm_read_byte(addr) (*(const uint8_t*)(addr))
#endif

#ifndef pgm_read_dword
#define pgm_read_dword(addr) (*(const uint32_t*)(addr))
#endif

// --- ESP System Stub ---
class ESPMock {
public:
    uint32_t getFreeHeap() { return 8388608; } // 8 MB mock desktop heap
    uint32_t getMaxAllocHeap() { return 4194304; }
};

static ESPMock ESP;

// --- Arduino String Alias ---
#ifndef Arduino_h
using String = std::string;
#endif

#endif // PLATFORM_LINUX
#endif // ARDUINO_LINUX_H
