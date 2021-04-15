#ifndef ADAFRUIT_EPD_H
#define ADAFRUIT_EPD_H

#define RAMBUFSIZE 64 ///< size of the ram buffer

// #include "adafruit_gfx.hpp"
#include "adafruit_platform.h"
#include <cmath>

enum {
    EPD_WHITE, ///< white color
    EPD_BLACK, ///< black color
    EPD_RED,   ///< red color
    EPD_GRAY,  ///< gray color ('red' on grayscale)
    EPD_DARK,  ///< darker color
    EPD_LIGHT, ///< lighter color
    EPD_NUM_COLORS
};

#define EPD_swap(a, b)                                                          \
    {                                                                           \
        int16_t t = a;                                                          \
        a = b;                                                                  \
        b = t;                                                                  \
    } ///< simple swap function

#define MCPSRAM_READ    0x03    ///< read command
#define MCPSRAM_WRITE   0x02    ///< write command
#define MCPSRAM_RDSR    0x05    ///< read status register command
#define MCPSRAM_WRSR    0x01    ///< write status register command

#define K640_SEQUENTIAL_MODE (1 << 6) ///< put ram chip in sequential mode

class Adafruit_EPD /*: public Adafruit_GFX*/ {
public:
    Adafruit_EPD(int width, int height, EPD *epd_, EPD_SPI_Handle *spi_);

    void begin(bool reset = true);
    void draw_pixel(int16_t x, int16_t y, uint16_t color);
    void clear_buffer();
    void clear_display();
    void set_black_buffer(int8_t index, bool inverted);
    void set_color_buffer(int8_t index, bool inverted);
    void display(bool sleep = false);

protected:
    void hardware_reset(void);
    void write_ram_framebuffer_to_epd(uint8_t *buffer, uint32_t buffer_size, uint8_t EPDlocation, bool invertdata = false);
    void write_sram_framebuffer_to_epd(uint16_t SRAM_buffer_addr, uint32_t buffer_size, uint8_t EPDlocation, bool invertdata = false);

    virtual uint8_t write_ram_command(uint8_t index) = 0;
    virtual void    set_ram_address(uint16_t x, uint16_t y) = 0;
    virtual void    busy_wait(void) = 0;
    virtual void    power_up() = 0;
    virtual void    update(void) = 0;
    virtual void    power_down(void) = 0;

    EPD             epd;
    EPD_SPI_Handle  *spi;

    uint16_t        default_refresh_delay = 15000;
    uint8_t         partialsSinceLastFullUpdate = 0;

    bool            blackInverted;  ///< is black channel inverted
    bool            colorInverted;  ///< is red channel inverted

    uint8_t         layer_colors[EPD_NUM_COLORS];

    uint32_t buffer1_size; ///< size of the primary buffer
    uint32_t buffer2_size; ///< size of the secondary buffer
    uint8_t *buffer1; ///< the pointer to the primary buffer if using on-chip ram
    uint8_t *buffer2; ///< the pointer to the secondary buffer if using on-chip ram
    uint8_t *color_buffer; ///< the pointer to the color buffer if using on-chip ram
    uint8_t *black_buffer; ///< the pointer to the black buffer if using on-chip ram
    uint16_t buffer1_addr; ///< The SRAM address offsets for the primary buffer
    uint16_t buffer2_addr; ///< The SRAM address offsets for the secondary buffer
    uint16_t colorbuffer_addr; ///< The SRAM address offsets for the color buffer
    uint16_t blackbuffer_addr; ///< The SRAM address offsets for the black buffer

    uint8_t EPD_command(uint8_t c, bool end = true);
    void    EPD_command(uint8_t c, const uint8_t *buf, uint16_t len);
    void    EPD_data(const uint8_t *buf, uint16_t len);
    void    EPD_data(uint8_t data);

    // SECTION GFX

    int16_t WIDTH;        ///< This is the 'raw' display width - never changes
    int16_t HEIGHT;       ///< This is the 'raw' display height - never changes
    int16_t _width;       ///< Display width as modified by current rotation
    int16_t _height;      ///< Display height as modified by current rotation
    int16_t cursor_x;     ///< x location to start print()ing text
    int16_t cursor_y;     ///< y location to start print()ing text
    uint16_t textcolor;   ///< 16-bit background color for print()
    uint16_t textbgcolor; ///< 16-bit text color for print()
    uint8_t textsize_x;   ///< Desired magnification in X-axis of text to print()
    uint8_t textsize_y;   ///< Desired magnification in Y-axis of text to print()
    uint8_t rotation;     ///< Display rotation (0 thru 3)
    bool wrap;            ///< If set, 'wrap' text at right edge of display
    bool _cp437;          ///< If set, use correct CP437 charset (default is off)

    int16_t width(void) const       { return _width; };
    int16_t height(void) const      { return _height; }
    uint8_t get_rotation(void) const { return rotation; }

    void set_rotation(uint8_t x) 
    {
        rotation = (x & 3);
        switch (rotation) {
            case 0:
            case 2:
                _width = WIDTH;
                _height = HEIGHT;
                break;
            case 1:
            case 3:
                _width = HEIGHT;
                _height = WIDTH;
                break;
        }
    }
public:
#ifndef _swap_int16_t
#define _swap_int16_t(a, b)                                                    \
  {                                                                            \
    int16_t t = a;                                                             \
    a = b;                                                                     \
    b = t;                                                                     \
  }
#endif

    // NOTE
    void write_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) 
    {
    #if defined(ESP8266)
        yield();
    #endif
        int16_t steep = abs(y1 - y0) > abs(x1 - x0);
        if (steep) {
            _swap_int16_t(x0, y0);
            _swap_int16_t(x1, y1);
        }

        if (x0 > x1) {
            _swap_int16_t(x0, x1);
            _swap_int16_t(y0, y1);
        }

        int16_t dx, dy;
        dx = x1 - x0;
        dy = abs(y1 - y0);

        int16_t err = dx / 2;
        int16_t ystep;

        if (y0 < y1) {
            ystep = 1;
        } else {
            ystep = -1;
        }

        for (; x0 <= x1; x0++) {
            if (steep) {
                draw_pixel(y0, x0, color);
            } else {
                draw_pixel(x0, y0, color);
            }
            err -= dy;
            if (err < 0) {
                y0 += ystep;
                err += dx;
            }
        }
    }

    void draw_fast_v_line(int16_t x, int16_t y, int16_t h, uint16_t color) 
    {
        write_line(x, y, x, y + h - 1, color);
    }

    void draw_fast_h_line(int16_t x, int16_t y, int16_t w, uint16_t color) 
    {
        write_line(x, y, x + w - 1, y, color);
    }

    void draw_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) 
    {
        draw_fast_h_line(x, y, w, color);
        draw_fast_h_line(x, y + h - 1, w, color);
        draw_fast_v_line(x, y, h, color);
        draw_fast_v_line(x + w - 1, y, h, color);
    }

    void fill_rect(int16_t x, int16_t y, int16_t w, int16_t h,uint16_t color) 
    {
        for (int16_t i = x; i < x + w; i++)
            draw_fast_v_line(i, y, h, color);
    }

    // !SECTION GFX
protected:
    // SECTION MCPSRAM

    void sram_begin();
    void sram_write(uint16_t addr, uint8_t *buf, uint16_t num);
    void sram_read(uint16_t addr, uint8_t *buf, uint16_t num);
    void sram_erase(uint16_t addr, uint16_t length, uint8_t val);
    
    // !SECTION MCPSRAM
};

#include "adafruit_ssd1608.h"

#endif // ADAFRUIT_EPD_H