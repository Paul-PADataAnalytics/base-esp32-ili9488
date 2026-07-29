// User_Setup.h for ESP32 WROOM + ILI9488 SPI Display
// Place this inside Arduino/libraries/TFT_eSPI/User_Setup.h or use via PlatformIO build_flags

#define ILI9488_DRIVER     // WARNING: Do not connect ILI9488 SDO/MISO directly without tri-state buffer if sharing SPI bus

#define TFT_MISO 19        // SDO / MISO
#define TFT_MOSI 23        // SDI / MOSI
#define TFT_SCLK 18        // SCK / CLK
#define TFT_CS    5        // Chip select control pin
#define TFT_DC   27        // Data Command control pin (GPIO27 safe non-strapping pin)
#define TFT_RST   4        // Reset pin (could connect to NodeMCU RST, see next line)
#define TFT_BL   32        // LED Backlight control pin (optional)

#define LOAD_GLCD   // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
#define LOAD_FONT2  // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
#define LOAD_FONT4  // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
#define LOAD_FONT6  // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only numbers
#define LOAD_FONT7  // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only numbers
#define LOAD_FONT8  // Font 8. Large 75 pixel font needs ~3256 bytes in FLASH, only numbers
#define LOAD_GFXFF  // FreeFonts. Include access to the 48 Adafruit_GFX free fonts FF1 to FF48 and custom fonts

#define SMOOTH_FONT

#define SPI_FREQUENCY  27000000 // 27MHz SPI speed (ILI9488 max reliable SPI frequency)
#define SPI_READ_FREQUENCY  20000000
