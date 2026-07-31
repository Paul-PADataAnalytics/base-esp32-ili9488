#ifndef DISPLAY_DRIVER_H
#define DISPLAY_DRIVER_H

#if !defined(PLATFORM_LINUX) && defined(ARDUINO)

#include <Arduino.h>
#include <LovyanGFX.hpp>

/**
 * Custom LGFX Panel class for ILI9488 SPI LCD + XPT2046 Touch Controller
 */
class LGFX_ILI9488 : public lgfx::LGFX_Device {
    lgfx::Panel_ILI9488  _panel_instance;
    lgfx::Bus_SPI        _bus_instance;
    lgfx::Touch_XPT2046  _touch_instance;

public:
    LGFX_ILI9488();
};

#endif

#endif // DISPLAY_DRIVER_H
