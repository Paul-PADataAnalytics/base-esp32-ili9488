#include "DisplayDriver.h"

LGFX_ILI9488::LGFX_ILI9488() {
    { // SPI Bus Configuration
        auto cfg = _bus_instance.config();
        cfg.spi_host   = VSPI_HOST;   // Use VSPI (SPI3) on ESP32
        cfg.spi_mode   = 0;
        cfg.freq_write = 27000000;    // 27 MHz SPI write speed
        cfg.freq_read  = 16000000;
        cfg.pin_sclk   = 18;          // Shared SCK (LCD CLK & Touch TCK)
        cfg.pin_mosi   = 23;          // Shared MOSI (LCD SDI & Touch TDI)
        cfg.pin_miso   = 19;          // Shared MISO (LCD SDO & Touch TDO)
        cfg.pin_dc     = 27;          // LCD DC pin
        _bus_instance.config(cfg);
        _panel_instance.setBus(&_bus_instance);
    }

    { // Panel Configuration
        auto cfg = _panel_instance.config();
        cfg.pin_cs           = 5;     // LCD CS pin
        cfg.pin_rst          = 4;     // LCD Reset pin
        cfg.pin_busy         = -1;
        cfg.panel_width      = 320;
        cfg.panel_height     = 480;
        cfg.offset_x         = 0;
        cfg.offset_y         = 0;
        cfg.offset_rotation  = 0;
        cfg.dummy_read_pixel = 8;
        cfg.readable         = true;
        cfg.invert           = false;
        cfg.rgb_order        = false;
        cfg.dlen_16bit       = false;
        cfg.bus_shared       = true;  // Shares SPI bus with touch controller
        _panel_instance.config(cfg);
    }

    { // Touch Controller Configuration (XPT2046)
        auto cfg = _touch_instance.config();
        cfg.x_min      = 300;
        cfg.x_max      = 3900;
        cfg.y_min      = 200;
        cfg.y_max      = 3700;
        cfg.pin_cs     = 33;          // TCS -> GPIO 33
        cfg.pin_int    = 36;          // PEN -> GPIO 36 (Interrupt)
        cfg.bus_shared = true;        // Shared VSPI bus with LCD
        cfg.offset_rotation = 0;
        cfg.spi_host   = VSPI_HOST;
        cfg.freq       = 2500000;     // 2.5 MHz Touch SPI frequency
        cfg.pin_sclk   = 18;          // TCK -> GPIO 18 (Shared SCK)
        cfg.pin_mosi   = 23;          // TDI -> GPIO 23 (Shared MOSI)
        cfg.pin_miso   = 19;          // TDO -> GPIO 19 (Shared MISO)
        _touch_instance.config(cfg);
        _panel_instance.setTouch(&_touch_instance);
    }

    { // Backlight Configuration
        auto cfg = _light_instance.config();
        cfg.pin_bl      = 32;         // LED Backlight pin
        cfg.invert      = false;
        cfg.freq        = 44100;
        cfg.pwm_channel = 7;
        _light_instance.config(cfg);
        _panel_instance.setLight(&_light_instance);
    }

    setPanel(&_panel_instance);
}
