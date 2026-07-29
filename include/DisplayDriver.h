#ifndef DISPLAY_DRIVER_H
#define DISPLAY_DRIVER_H

#include <Arduino.h>
#include <LovyanGFX.hpp>

/**
 * Custom LGFX Panel class for ILI9488 SPI LCD
 */
class LGFX_ILI9488 : public lgfx::LGFX_Device {
    lgfx::Panel_ILI9488 _panel_instance;
    lgfx::Bus_SPI       _bus_instance;
    lgfx::Light_PWM     _light_instance;

public:
    LGFX_ILI9488();
};

// Singleton display hardware instance declaration
extern LGFX_ILI9488 tft_driver;

#endif // DISPLAY_DRIVER_H
