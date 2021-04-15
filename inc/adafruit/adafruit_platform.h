#ifndef ADAFRUIT_PLATFORM_H
#define ADAFRUIT_PLATFORM_H

#include <cstdint>
#include "stm32l4xx_hal.h"

#define EPD_SPI_TIMEOUT_MS  1000
#define EPD_SPI_CLOCK_FREQ  4000000

// Definitions of platform specific pin states
#define EPD_PIN_LOW         GPIO_PIN_RESET
#define EPD_PIN_HIGH        GPIO_PIN_SET

// Typedef for platform specific result status type
typedef HAL_StatusTypeDef   EPD_Status;

// Typedef for platform specific SPI handle
typedef SPI_HandleTypeDef   EPD_SPI_Handle;

// Typedef for platform specific pin state type
typedef GPIO_PinState       EPD_PinState;

// Structure to handle port and pin number at the same time
typedef struct {
    GPIO_TypeDef    *port;
    uint16_t        pin;
} EPD_Pin;

// Structure to hold all relevant display settings
typedef struct {
    EPD_Pin cs;
    EPD_Pin dc;
    EPD_Pin srcs;
    EPD_Pin sdcs;
    EPD_Pin rst;
    EPD_Pin busy;
    EPD_Pin ena;
} EPD;

void            EPD_Delay(uint32_t ms);
bool            EPD_PinValid(EPD_Pin pin);
void            EPD_PinWrite(EPD_Pin pin, EPD_PinState state);
EPD_PinState    EPD_PinRead(EPD_Pin pin);

EPD_Status      EPD_SPI_Tx(EPD_SPI_Handle *spi, const uint8_t *buf, size_t len, uint32_t timeout);
EPD_Status      EPD_SPI_Rx(EPD_SPI_Handle *spi, uint8_t *buf, size_t len, uint32_t timeout);
EPD_Status      EPD_SPI_TxRx(EPD_SPI_Handle *spi, uint8_t *tx_buf, uint8_t *rx_buf, size_t len, uint32_t timeout);

// NOTE Below are only unnecessary functions. 
// Not used anywere in library. Only for your own convenience

void            EPD_SPI_Init(EPD_SPI_Handle *spi);
void            EPD_SPI_DeInit(EPD_SPI_Handle *spi);
void            EPD_GPIO_Init(EPD *epd);
void            EPD_GPIO_DeInit(EPD *epd);

#endif // ADAFRUIT_PLATFORM_H