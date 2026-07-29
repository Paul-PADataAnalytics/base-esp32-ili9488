# 📌 ESP32 WROOM to 3.5" ILI9488 TFT LCD + XPT2046 Touch Pinout Guide

This document defines the complete hardware pin mapping for connecting an **ESP32 WROOM-32** development board to a **3.5" ILI9488 SPI TFT LCD Display** featuring an integrated **XPT2046 Resistive Touch Controller**.

---

## 🔌 Hardware Pin Mapping Table

### 1. TFT LCD Display Controller (ILI9488)
| LCD Pin Label | ESP32 GPIO Pin | Description | Signal Type |
|---|---|---|---|
| **VCC** | **VIN / 5V** | 5V Main Power Input | Power |
| **GND** | **GND** | Common Ground | Power Ground |
| **CS** | **GPIO 5** | LCD Chip Select (Active LOW) | Output |
| **RESET** | **GPIO 4** | Hardware LCD Reset (Active LOW) | Output |
| **DC / RS** | **GPIO 27** | Data / Command Selection | Output |
| **SDI (MOSI)** | **GPIO 23** | SPI Master Out Slave In | Shared SPI Output |
| **SCK (CLK)** | **GPIO 18** | SPI Bus Serial Clock | Shared SPI Clock |
| **LED** | **GPIO 32** | Display Backlight Power (High = On) | Output |
| **SDO (MISO)** | **GPIO 19** | SPI Master In Slave Out | Shared SPI Input |

### 2. Touch Screen Controller (XPT2046)
| Touch Pin Label | ESP32 GPIO Pin | Connection Strategy | Signal Type |
|---|---|---|---|
| **TCK** | **GPIO 18** | **Shared SCK** (Connect to LCD SCK / GPIO 18) | Shared SPI Clock |
| **TCS** | **GPIO 33** | **Dedicated Touch CS** (Connect to GPIO 33) | Output |
| **TDI** | **GPIO 23** | **Shared MOSI** (Connect to LCD MOSI / GPIO 23) | Shared SPI Output |
| **TDO** | **GPIO 19** | **Shared MISO** (Connect to LCD MISO / GPIO 19) | Shared SPI Input |
| **PEN** | **GPIO 36** | **Touch Interrupt (IRQ)** (Connect to GPIO 36) | Input (Active LOW) |

---

## ⚡ Important Hardware Wiring & SPI Bus Notes

1. **Shared VSPI Bus Architecture**:
   - To conserve GPIO pins on the ESP32, **TCK**, **TDI**, and **TDO** share the exact same SPI bus lines (**GPIO 18**, **GPIO 23**, **GPIO 19**) as the ILI9488 display.
   - Only **TCS** (**GPIO 33**) requires a dedicated Chip Select output line.

2. **Strapping Pin Safety**:
   - **GPIO 2** is an ESP32 strapping pin required for serial bootloader flashing (`esptool.py`). Using GPIO 2 for LCD DC causes serial flash header failures (`invalid header: 0xffffffff`).
   - **GPIO 27** is used for **TFT_DC** to ensure 100% reliable firmware uploading.

3. **MISO Bus Contention Prevention**:
   - On breadboard wiring harnesses, unbuffered XPT2046 touch controllers can hold the MISO line active when idle.
   - The framework configures the ILI9488 panel in **Write-Only SPI mode** (`pin_miso = -1`), eliminating bus contention between the touch chip and display controller.

4. **Touch CS Initial State**:
   - **GPIO 33** (`TCS`) is driven `HIGH` immediately upon `setup()` startup to ensure the touch controller is deselected during display initialization.

---

## 🗺️ Visual Wiring Diagram

```text
    ESP32 WROOM-32                     3.5" ILI9488 TFT LCD + Touch
 ┌───────────────────┐                     ┌────────────────────┐
 │               VIN ├────────────────────►│ VCC (5V)           │
 │               GND ├────────────────────►│ GND                │
 │            GPIO 5 ├────────────────────►│ CS (LCD)           │
 │            GPIO 4 ├────────────────────►│ RESET              │
 │           GPIO 27 ├────────────────────►│ DC / RS            │
 │           GPIO 32 ├────────────────────►│ LED (Backlight)    │
 │           GPIO 33 ├────────────────────►│ TCS (Touch CS)     │
 │           GPIO 36 ├◄───────────────────┤ PEN (Touch IRQ)    │
 │                   │                     │                    │
 │           GPIO 18 ├──┬─────────────────►│ SCK (LCD Clock)    │
 │     (Shared SCK)  │  └─────────────────►│ TCK (Touch Clock)  │
 │                   │                     │                    │
 │           GPIO 23 ├──┬─────────────────►│ SDI (LCD MOSI)     │
 │    (Shared MOSI)  │  └─────────────────►│ TDI (Touch MOSI)   │
 │                   │                     │                    │
 │           GPIO 19 ├──┬─────────────────►│ SDO (LCD MISO)     │
 │    (Shared MISO)  │  └─────────────────►│ TDO (Touch MISO)   │
 └───────────────────┘                     └────────────────────┘
```
