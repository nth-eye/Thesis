#include "adafruit_epd.h"
#include <cstdlib>
#include <cstring>

Adafruit_EPD::Adafruit_EPD(int width, int height, EPD *epd_, EPD_SPI_Handle *spi_)
    : /*Adafruit_GFX(width, height),*/ epd(*epd_)
{
    // SECTION GFX

    WIDTH = width;
    HEIGHT = height;
    _width = WIDTH;
    _height = HEIGHT;
    rotation = 0;
    cursor_y = cursor_x = 0;
    textsize_x = textsize_y = 1;
    textcolor = textbgcolor = 0xFFFF;
    wrap = true;
    _cp437 = false;

    // !SECTION GFX

    spi = spi_;

    buffer1_size = buffer2_size = 0;
    buffer1_addr = buffer2_addr = 0;
    colorbuffer_addr = blackbuffer_addr = 0;
    buffer1 = buffer2 = color_buffer = black_buffer = NULL;
}

void Adafruit_EPD::begin(bool reset) 
{
    set_black_buffer(0, true);  // black defaults to inverted
    set_color_buffer(1, false); // red defaults to not inverted

    layer_colors[EPD_WHITE] = 0b00;
    layer_colors[EPD_BLACK] = 0b01;
    layer_colors[EPD_RED] = 0b10;
    layer_colors[EPD_GRAY] = 0b10;
    layer_colors[EPD_DARK] = 0b01;
    layer_colors[EPD_LIGHT] = 0b10;

    // EPD_GPIO_Init(&epd);    
    // EPD_SPI_Init(spi);

    if (EPD_PinValid(epd.srcs)) 
        sram_begin();

    EPD_PinWrite(epd.cs, EPD_PIN_HIGH);

    if (reset)
        hardware_reset();
}

void Adafruit_EPD::hardware_reset(void) 
{
    if (EPD_PinValid(epd.rst)) {
        // VDD (3.3V) goes high at start, lets just chill for a ms
        EPD_PinWrite(epd.rst, EPD_PIN_HIGH);
        EPD_Delay(10);
        // bring reset low
        EPD_PinWrite(epd.rst, EPD_PIN_LOW);
        // wait 10ms
        EPD_Delay(10);
        // bring out of reset
        EPD_PinWrite(epd.rst, EPD_PIN_HIGH);
        EPD_Delay(10);
    }
}

void Adafruit_EPD::draw_pixel(int16_t x, int16_t y, uint16_t color) 
{
    if ((x < 0) || (x >= width()) || (y < 0) || (y >= height()))
        return;

    uint8_t *black_pBuf, *color_pBuf;

    // deal with non-8-bit heights
    uint16_t _HEIGHT = HEIGHT;

    if (_HEIGHT % 8 != 0)
        _HEIGHT += 8 - (_HEIGHT % 8);

    // check rotation, move pixel around if necessary
    switch (get_rotation()) {
    case 1:
        EPD_swap(x, y);
        x = WIDTH - x - 1;
        break;
    case 2:
        x = WIDTH - x - 1;
        y = _HEIGHT - y - 1;
        break;
    case 3:
        EPD_swap(x, y);
        y = _HEIGHT - y - 1;
        break;
    }
    uint16_t addr = ((uint32_t)(WIDTH - 1 - x) * (uint32_t)_HEIGHT + y) / 8;
    uint8_t black_c, color_c;

    if (EPD_PinValid(epd.srcs)) {
        sram_read(blackbuffer_addr + addr, &black_c, 1);
        black_pBuf = &black_c;
        sram_read(colorbuffer_addr + addr, &color_c, 1);
        color_pBuf = &color_c;
    } else {
        color_pBuf = color_buffer + addr;
        black_pBuf = black_buffer + addr;
    }

    bool color_bit, black_bit;

    black_bit = layer_colors[color] & 0x1;
    color_bit = layer_colors[color] & 0x2;

    if ((color_bit && colorInverted) || (!color_bit && !colorInverted))
        *color_pBuf &= ~(1 << (7 - y % 8));
    else
        *color_pBuf |= (1 << (7 - y % 8));

    if ((black_bit && blackInverted) || (!black_bit && !blackInverted))
        *black_pBuf &= ~(1 << (7 - y % 8));
    else
        *black_pBuf |= (1 << (7 - y % 8));

    if (EPD_PinValid(epd.srcs)) {
        sram_write(colorbuffer_addr + addr, color_pBuf, 1);
        sram_write(blackbuffer_addr + addr, black_pBuf, 1);
    }
}

void Adafruit_EPD::write_ram_framebuffer_to_epd(uint8_t *framebuffer, uint32_t framebuffer_size, uint8_t EPDlocation, bool invertdata) 
{
    // write image
    write_ram_command(EPDlocation);
    EPD_PinWrite(epd.dc, EPD_PIN_HIGH);

    // if (invertdata) {
    //     for (int i = 0; i < framebuffer_size; ++i)
    //         framebuffer[i] = ~framebuffer[i];
    // }

    for (size_t i = 0; i < framebuffer_size; ++i) {

        uint8_t d = framebuffer[i];

        if (invertdata)
            d = ~d;

        EPD_SPI_Tx(spi, &d, 1, EPD_SPI_TIMEOUT_MS);
    }
    EPD_PinWrite(epd.cs, EPD_PIN_HIGH);
}

void Adafruit_EPD::write_sram_framebuffer_to_epd(uint16_t SRAM_buffer_addr, uint32_t buffer_size, uint8_t EPDlocation, bool invertdata) 
{
    // use SRAM
    EPD_PinWrite(epd.srcs, EPD_PIN_LOW);

    uint8_t buf[3] = { MCPSRAM_READ, SRAM_buffer_addr >> 8, SRAM_buffer_addr & 0xff };

    EPD_SPI_Tx(spi, buf, 3, EPD_SPI_TIMEOUT_MS);

    // first data byte from SRAM will be transfered in at the same time
    // as the EPD command is transferred out
    uint8_t c = write_ram_command(EPDlocation);

    EPD_PinWrite(epd.dc, EPD_PIN_HIGH);

    for (size_t i = 0; i < buffer_size; ++i)
        EPD_SPI_TxRx(spi, &c, &c, 1, EPD_SPI_TIMEOUT_MS);

    EPD_PinWrite(epd.cs, EPD_PIN_HIGH);
    EPD_PinWrite(epd.srcs, EPD_PIN_HIGH);
}

void Adafruit_EPD::display(bool sleep) 
{
    power_up();

    // Set X & Y ram counters
    set_ram_address(0, 0);

    if (EPD_PinValid(epd.srcs))
        write_sram_framebuffer_to_epd(buffer1_addr, buffer1_size, 0);
    else
        write_ram_framebuffer_to_epd(buffer1, buffer1_size, 0);


    if (buffer2_size != 0) {
        // oh there's another buffer eh?
        EPD_Delay(2);

        // Set X & Y ram counters
        set_ram_address(0, 0);

        if (EPD_PinValid(epd.srcs))
            write_sram_framebuffer_to_epd(buffer2_addr, buffer2_size, 1);
        else
            write_ram_framebuffer_to_epd(buffer2, buffer2_size, 1);
    }

    update();
    partialsSinceLastFullUpdate = 0;

    if (sleep)
        power_down();
}

void Adafruit_EPD::set_black_buffer(int8_t index, bool inverted) 
{
    if (index == 0) {
        if (EPD_PinValid(epd.srcs))
            blackbuffer_addr = buffer1_addr;
        else
            black_buffer = buffer1;
    }
    if (index == 1) {
        if (EPD_PinValid(epd.srcs))
            blackbuffer_addr = buffer2_addr;
        else
            black_buffer = buffer2;
    }
    blackInverted = inverted;
}

void Adafruit_EPD::set_color_buffer(int8_t index, bool inverted) 
{
    if (index == 0) {
        if (EPD_PinValid(epd.srcs))
            colorbuffer_addr = buffer1_addr;
        else
            color_buffer = buffer1;
    }
    if (index == 1) {
        if (EPD_PinValid(epd.srcs))
            colorbuffer_addr = buffer2_addr;
        else
            color_buffer = buffer2;
    }
    colorInverted = inverted;
}

void Adafruit_EPD::clear_buffer() 
{
    if (EPD_PinValid(epd.srcs)) {
        sram_erase(blackbuffer_addr, buffer1_size, blackInverted ? 0xff : 0x00);
        sram_erase(colorbuffer_addr, buffer2_size, colorInverted ? 0xff : 0x00);
    } else {

        if (black_buffer)
            memset(black_buffer, blackInverted ? 0xff : 0x00, buffer1_size);

        if (color_buffer)
            memset(color_buffer, colorInverted ? 0xff : 0x00, buffer2_size);
    }
}

void Adafruit_EPD::clear_display() 
{
    clear_buffer();
    display();
    EPD_Delay(100);
    display();
}

void Adafruit_EPD::EPD_command(uint8_t c, const uint8_t *buf, uint16_t len) 
{
    EPD_command(c, false);
    EPD_data(buf, len);
}

uint8_t Adafruit_EPD::EPD_command(uint8_t c, bool end) 
{
    EPD_PinWrite(epd.cs, EPD_PIN_HIGH);
    EPD_PinWrite(epd.dc, EPD_PIN_LOW);
    EPD_PinWrite(epd.cs, EPD_PIN_LOW);

    EPD_SPI_TxRx(spi, &c, &c, 1, EPD_SPI_TIMEOUT_MS);
    
    if (end)
        EPD_PinWrite(epd.cs, EPD_PIN_HIGH);

    return c;
}

void Adafruit_EPD::EPD_data(const uint8_t *buf, uint16_t len) 
{
    EPD_PinWrite(epd.dc, EPD_PIN_HIGH);
    EPD_SPI_Tx(spi, buf, len, EPD_SPI_TIMEOUT_MS);
    EPD_PinWrite(epd.cs, EPD_PIN_HIGH);
}

void Adafruit_EPD::EPD_data(uint8_t data) 
{
    EPD_PinWrite(epd.cs, EPD_PIN_HIGH);
    EPD_PinWrite(epd.dc, EPD_PIN_HIGH);
    EPD_PinWrite(epd.cs, EPD_PIN_LOW);
    EPD_SPI_Tx(spi, &data, 1, EPD_SPI_TIMEOUT_MS);
    EPD_PinWrite(epd.cs, EPD_PIN_HIGH);
}

void Adafruit_EPD::sram_begin() 
{
    EPD_PinWrite(epd.srcs, EPD_PIN_LOW);

    uint8_t buf[] = { 0xff, 0xff, 0xff, MCPSRAM_WRSR, K640_SEQUENTIAL_MODE };

    EPD_SPI_Tx(spi, buf, 5, EPD_SPI_TIMEOUT_MS);

    EPD_PinWrite(epd.srcs, EPD_PIN_HIGH);
}

void Adafruit_EPD::sram_write(uint16_t addr, uint8_t *buf, uint16_t len) 
{
    EPD_PinWrite(epd.srcs, EPD_PIN_LOW);

    uint8_t cmd[3] = { MCPSRAM_WRITE, addr >> 8, addr & 0xff };

    EPD_SPI_Tx(spi, cmd, 3, EPD_SPI_TIMEOUT_MS);
    EPD_SPI_Tx(spi, buf, len, EPD_SPI_TIMEOUT_MS);

    EPD_PinWrite(epd.srcs, EPD_PIN_HIGH);
}

void Adafruit_EPD::sram_read(uint16_t addr, uint8_t *buf, uint16_t len) 
{
    EPD_PinWrite(epd.srcs, EPD_PIN_LOW);

    uint8_t cmd[3] = { MCPSRAM_READ, addr >> 8, addr & 0xff };

    EPD_SPI_Tx(spi, cmd, 3, EPD_SPI_TIMEOUT_MS);
    EPD_SPI_Rx(spi, buf, len, EPD_SPI_TIMEOUT_MS);

    EPD_PinWrite(epd.srcs, EPD_PIN_HIGH);
}

void Adafruit_EPD::sram_erase(uint16_t addr, uint16_t len, uint8_t val) 
{
    EPD_PinWrite(epd.srcs, EPD_PIN_LOW);

    uint8_t cmdbuf[3] = { MCPSRAM_WRITE, addr >> 8, addr & 0xff };

    EPD_SPI_Tx(spi, cmdbuf, 3, EPD_SPI_TIMEOUT_MS);
    for (uint16_t i = 0; i < len; ++i) 
        EPD_SPI_Tx(spi, &val, 1, EPD_SPI_TIMEOUT_MS);

    EPD_PinWrite(epd.srcs, EPD_PIN_HIGH);
}
