#include "adafruit_epd.h"
#include "adafruit_ssd1608.h"

#define BUSY_WAIT 500

#define SSD1608_DRIVER_CONTROL      0x01
#define SSD1608_GATE_VOLTAGE        0x03
#define SSD1608_SOURCE_VOLTAGE      0x04
#define SSD1608_DISPLAY_CONTROL     0x07
#define SSD1608_NON_OVERLAP         0x0B
#define SSD1608_BOOSTER_SOFT_START  0x0C
#define SSD1608_GATE_SCAN_START     0x0F
#define SSD1608_DEEP_SLEEP          0x10
#define SSD1608_DATA_MODE           0x11
#define SSD1608_SW_RESET            0x12
#define SSD1608_TEMP_WRITE          0x1A
#define SSD1608_TEMP_READ           0x1B
#define SSD1608_TEMP_CONTROL        0x1C
#define SSD1608_TEMP_LOAD           0x1D
#define SSD1608_MASTER_ACTIVATE     0x20
#define SSD1608_DISP_CTRL1          0x21
#define SSD1608_DISP_CTRL2          0x22
#define SSD1608_WRITE_RAM           0x24
#define SSD1608_READ_RAM            0x25
#define SSD1608_VCOM_SENSE          0x28
#define SSD1608_VCOM_DURATION       0x29
#define SSD1608_WRITE_VCOM          0x2C
#define SSD1608_READ_OTP            0x2D
#define SSD1608_WRITE_LUT           0x32
#define SSD1608_WRITE_DUMMY         0x3A
#define SSD1608_WRITE_GATELINE      0x3B
#define SSD1608_WRITE_BORDER        0x3C
#define SSD1608_SET_RAMXPOS         0x44
#define SSD1608_SET_RAMYPOS         0x45
#define SSD1608_SET_RAMXCOUNT       0x4E
#define SSD1608_SET_RAMYCOUNT       0x4F
#define SSD1608_NOP                 0xFF

const unsigned char LUT_DATA[30] = {
    0x02, 0x02, 0x01, 0x11, 0x12, 0x12, 0x22, 0x22, 0x66, 0x69,
    0x69, 0x59, 0x58, 0x99, 0x99, 0x88, 0x00, 0x00, 0x00, 0x00,
    0xF8, 0xB4, 0x13, 0x51, 0x35, 0x51, 0x51, 0x19, 0x01, 0x00
};

Adafruit_SSD1608::Adafruit_SSD1608(int width, int height, EPD *epd_, EPD_SPI_Handle *spi/*, uint8_t *buf*/)
    : Adafruit_EPD(width, height, epd_, spi) 
{
    if ((height % 8) != 0)
        height += 8 - (height % 8);

    buffer1_size = (uint16_t) width * (uint16_t) height / 8;
    buffer2_size = 0;

    if (EPD_PinValid(epd.srcs)) {
        buffer1_addr = 0;
        buffer2_addr = 0;
    } else {
        buffer1 = NULL; // (uint8_t*) malloc(buffer1_size);
        buffer2 = buffer1;
    }
}

void Adafruit_SSD1608::busy_wait(void) 
{
    if (EPD_PinValid(epd.busy)) {
        while (EPD_PinRead(epd.busy))
            EPD_Delay(10);
    } else {
        EPD_Delay(BUSY_WAIT);
    }
}

void Adafruit_SSD1608::begin(bool reset) 
{
    Adafruit_EPD::begin(reset);
    set_black_buffer(0, true); // black defaults to inverted
    set_color_buffer(0, true); // no secondary buffer, so we'll just reuse index 0

    EPD_Delay(100);
    power_down();
}

void Adafruit_SSD1608::update() 
{
    uint8_t buf = 0xc7;

    EPD_command(SSD1608_DISP_CTRL2, &buf, 1);
    EPD_command(SSD1608_MASTER_ACTIVATE);

    busy_wait();
}

void Adafruit_SSD1608::power_up() 
{
    uint8_t buf[5];

    hardware_reset();
    busy_wait();

    // soft reset
    EPD_command(SSD1608_SW_RESET);
    busy_wait();

    // driver output control
    buf[0] = HEIGHT - 1;
    buf[1] = (HEIGHT - 1) >> 8;
    buf[2] = 0x00;
    EPD_command(SSD1608_DRIVER_CONTROL, buf, 3);

    // Set dummy line period
    buf[0] = 0x1B;
    EPD_command(SSD1608_WRITE_DUMMY, buf, 1);

    // Set gate line width
    buf[0] = 0x0B;
    EPD_command(SSD1608_WRITE_GATELINE, buf, 1);

    // Data entry sequence
    buf[0] = 0x03;
    EPD_command(SSD1608_DATA_MODE, buf, 1);

    // Set ram X start/end postion
    buf[0] = 0x00;
    buf[1] = WIDTH / 8 - 1;
    EPD_command(SSD1608_SET_RAMXPOS, buf, 2);

    // Set ram Y start/end postion
    buf[0] = 0x00;
    buf[1] = 0x00;
    buf[2] = HEIGHT - 1;
    buf[3] = (HEIGHT - 1) >> 8;
    EPD_command(SSD1608_SET_RAMYPOS, buf, 4);

    // Vcom Voltage
    buf[0] = 0x70;
    EPD_command(SSD1608_WRITE_VCOM, buf, 1);

    EPD_command(SSD1608_WRITE_LUT, LUT_DATA, 30);

    busy_wait();
}

void Adafruit_SSD1608::power_down(void) 
{
    uint8_t buf = 0x01;

    EPD_command(SSD1608_DEEP_SLEEP, &buf, 1);
    EPD_Delay(100);
}

uint8_t Adafruit_SSD1608::write_ram_command(uint8_t index) 
{
    return EPD_command(SSD1608_WRITE_RAM, false);
}

void Adafruit_SSD1608::set_ram_address(uint16_t x, uint16_t y) 
{
    uint8_t buf[2];

    // Set RAM X address counter
    buf[0] = x;
    EPD_command(SSD1608_SET_RAMXCOUNT, buf, 1);

    // Set RAM Y address counter
    buf[0] = y >> 8;
    buf[1] = y;
    EPD_command(SSD1608_SET_RAMYCOUNT, buf, 2);
}
