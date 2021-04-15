#ifndef ADAFRUIT_154_MONO_D27_H
#define ADAFRUIT_154_MONO_D27_H

#include "adafruit_ssd1608.h"

class Adafruit_154_Mono_D27 : public Adafruit_SSD1608 {
public:
    Adafruit_154_Mono_D27(EPD *epd, EPD_SPI_Handle *spi)
        : Adafruit_SSD1608(200, 200, epd, spi/*, buf*/)
    {};

    void begin() 
    {
        Adafruit_SSD1608::begin(true);
        set_color_buffer(0, true); // layer 0 uninverted
        set_black_buffer(0, true); // only one buffer

        layer_colors[EPD_WHITE] = 0b00;
        layer_colors[EPD_BLACK] = 0b01;
        layer_colors[EPD_RED] = 0b01;
        layer_colors[EPD_GRAY] = 0b01;
        layer_colors[EPD_LIGHT] = 0b00;
        layer_colors[EPD_DARK] = 0b01;

        default_refresh_delay = 1000;
        set_rotation(3);
        power_down();
    }
private:
    //uint8_t buf[(uint16_t) 200 * (uint16_t) 200 / 8];
};

#endif // ADAFRUIT_154_MONO_D27_H
