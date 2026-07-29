#include "DisplayDriver.h"

LGFX_ILI9488::LGFX_ILI9488() {
    { // SPI Bus Configuration
        auto cfg = _bus_instance.config();
        cfg.spi_host   = VSPI_HOST;   // Use VSPI (SPI3) on ESP32
        cfg.spi_mode   = 0;
        cfg.freq_write = 27000000;    // 27 MHz SPI write speed
        cfg.freq_read  = 16000000;
        cfg.pin_sclk   = 18;          // SCK pin
        cfg.pin_mosi   = 23;          // MOSI pin
        cfg.pin_miso   = 19;          // MISO pin
        cfg.pin_dc     = 27;          // DC pin
        _bus_instance.config(cfg);
        _panel_instance.setBus(&_bus_instance);
    }

    { // Panel Configuration
        auto cfg = _panel_instance.config();
        cfg.pin_cs           = 5;     // CS pin
        cfg.pin_rst          = 4;     // Reset pin
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
        cfg.bus_shared       = true;
        _panel_instance.config(cfg);
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

// Single global display driver instance definition
LGFX_ILI9488 tft_driver;
