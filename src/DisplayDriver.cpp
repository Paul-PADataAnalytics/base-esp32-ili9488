#include "DisplayDriver.h"

LGFX_ILI9488::LGFX_ILI9488() {
    { // SPI Bus Configuration
        auto cfg = _bus_instance.config();
        cfg.spi_host   = VSPI_HOST;   // Use VSPI (SPI3) on ESP32
        cfg.spi_mode   = 0;
        cfg.freq_write = 20000000;    // 20 MHz SPI write speed (Rock-solid for breadboard wiring!)
        cfg.freq_read  = 16000000;
        cfg.pin_sclk   = 18;          // SCK pin (LCD CLK & Touch TCK)
        cfg.pin_mosi   = 23;          // MOSI pin (LCD SDI & Touch TDI)
        cfg.pin_miso   = -1;          // LCD Write-Only (Prevents MISO bus contention from Touch TDO!)
        cfg.pin_dc     = 27;          // DC pin
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
        cfg.readable         = false; // Write-only mode prevents MISO stalls
        cfg.invert           = false;
        cfg.rgb_order        = false;
        cfg.dlen_16bit       = false;
        cfg.bus_shared       = true;  // Shares SPI bus with touch controller
        _panel_instance.config(cfg);
    }

    { // Touch Controller Configuration (XPT2046 on Shared Bus)
        auto cfg = _touch_instance.config();
        cfg.x_min      = 3900;        // Inverted x_min & x_max to correct mirrored X axis!
        cfg.x_max      = 300;
        cfg.y_min      = 200;
        cfg.y_max      = 3700;
        cfg.pin_cs     = 33;          // TCS -> GPIO 33
        cfg.pin_int    = 36;          // PEN -> GPIO 36
        cfg.spi_host   = VSPI_HOST;   // Target VSPI_HOST explicitly
        cfg.bus_shared = true;        // Inherits SPI bus from panel (VSPI_HOST)
        cfg.freq       = 1000000;     // 1 MHz Touch SPI frequency
        cfg.pin_sclk   = 18;          // TCK -> GPIO 18
        cfg.pin_mosi   = 23;          // TDI -> GPIO 23
        cfg.pin_miso   = 19;          // TDO -> GPIO 19
        _touch_instance.config(cfg);
        _panel_instance.setTouch(&_touch_instance);
    }

    setPanel(&_panel_instance);
}
