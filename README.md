# 🚀 ESP32 WROOM + ILI9488 3.5" TFT LCD + XPT2046 Touch HAL Framework

A high-performance, double-buffered, hardware-abstracted graphics & touch framework for ESP32 and ILI9488 3.5" TFT displays.

---

## 📌 Complete Pinout Table

### 1. Display Pins (ILI9488 LCD Driver)
| LCD Pin Label | ESP32 GPIO Pin | Function |
|---|---|---|
| **VCC** | **VIN (5V)** | Power Supply |
| **GND** | **GND** | Ground |
| **CS** | **GPIO 5** | LCD Chip Select |
| **RESET** | **GPIO 4** | LCD Hardware Reset |
| **DC / RS** | **GPIO 27** | Data / Command Selection |
| **SDI (MOSI)** | **GPIO 23** | SPI Master Out Slave In |
| **SCK (CLK)** | **GPIO 18** | SPI Clock |
| **LED** | **GPIO 32** | Backlight Control |
| **SDO (MISO)** | **GPIO 19** | SPI Master In Slave Out |

### 2. Touch Screen Pins (XPT2046 Touch Controller)
| Touch Pin Label | ESP32 GPIO Pin | Connection Strategy |
|---|---|---|
| **TCK** | **GPIO 18** | **Shared SCK** (Connect to LCD SCK / GPIO 18) |
| **TCS** | **GPIO 33** | **Dedicated Touch CS** (Connect to GPIO 33) |
| **TDI** | **GPIO 23** | **Shared MOSI** (Connect to LCD MOSI / GPIO 23) |
| **TDO** | **GPIO 19** | **Shared MISO** (Connect to LCD MISO / GPIO 19) |
| **PEN** | **GPIO 36** | **Touch Interrupt** (Connect to GPIO 36 or leave optional) |

---

## 🎮 Interactive Touch Demo

When you touch or drag your finger on the display:
- **Touch Coordinates**: Displays real-time `Touch: (X, Y)` at top-right.
- **Interactive 3D Rotation**: Dragging rotates the 3D wireframe cube in real time.
- **Sphere Tracking**: The energy sphere locks to your touch location.

---

## 🏛️ Framework Application Template

To write a new touch-enabled program, inherit from `BaseApp`:

```cpp
#include "BaseApp.h"
#include "GFXContext.h"

class MyTouchApp : public BaseApp {
public:
    void setup(GFXContext& gfx) override {}

    void update(float deltaTime) override {}

    void render(GFXContext& gfx) override {
        int x, y;
        if (gfx.getTouch(&x, &y)) {
            gfx.drawCircleDirect(x, y, 15, 0x07FF, true);
        }
    }
};
```
