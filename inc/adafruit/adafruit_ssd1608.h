#ifndef ADAFRUIT_SSD1608_H
#define ADAFRUIT_SSD1608_H

#include "adafruit_epd.h"

class Adafruit_SSD1608 : public Adafruit_EPD {
public:
    Adafruit_SSD1608(int width, int height, EPD *epd_, EPD_SPI_Handle *spi/*, uint8_t *buf*/);

    void begin(bool reset = true);
    void power_up();
    void power_down();
    void update();

protected:
    uint8_t write_ram_command(uint8_t index);
    void set_ram_address(uint16_t x, uint16_t y);
    void busy_wait();
};

#endif // ADAFRUIT_SSD1608_H
