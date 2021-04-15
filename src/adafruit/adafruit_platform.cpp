#include "adafruit_platform.h"
#include "main.h"

EPD_PinState EPD_PinRead(EPD_Pin pin)
{
    return HAL_GPIO_ReadPin(pin.port, pin.pin);
}

void EPD_PinWrite(EPD_Pin pin, EPD_PinState state)
{
    HAL_GPIO_WritePin(pin.port, pin.pin, state);
}

bool EPD_PinValid(EPD_Pin pin)
{
    return pin.pin != 0;
}

void EPD_Delay(uint32_t ms)
{
    HAL_Delay(ms);
}

EPD_Status EPD_SPI_Tx(EPD_SPI_Handle *spi, const uint8_t *buf, size_t len, uint32_t timeout)
{
    return HAL_SPI_Transmit(spi, (uint8_t*) buf, len, timeout);
}

EPD_Status EPD_SPI_Rx(EPD_SPI_Handle *spi, uint8_t *buf, size_t len, uint32_t timeout)
{
    return HAL_SPI_Receive(spi, buf, len, timeout);
}

EPD_Status EPD_SPI_TxRx(EPD_SPI_Handle *spi, uint8_t *tx_buf, uint8_t *rx_buf, size_t len, uint32_t timeout)
{
    return HAL_SPI_TransmitReceive(spi, tx_buf, rx_buf, len, timeout);
}

void EPD_SPI_Init(EPD_SPI_Handle *spi)
{
    spi->State                  = HAL_SPI_STATE_RESET;

    spi->Instance               = SPI3;
    spi->Init.Mode              = SPI_MODE_MASTER;
    spi->Init.Direction         = SPI_DIRECTION_2LINES;
    spi->Init.DataSize          = SPI_DATASIZE_8BIT;
    spi->Init.CLKPolarity       = SPI_POLARITY_LOW;
    spi->Init.CLKPhase          = SPI_PHASE_1EDGE;
    spi->Init.NSS               = SPI_NSS_SOFT;

    // NOTE Little helper functionality to calculate appropriate prescaler 
    // for Adafruit display. Frequency must be approximately 4 MHz. Use it 
    // if you don't know which one to choose -> hspi3.Init.BaudRatePrescaler = prescalers[i];
    // uint32_t freq = HAL_RCC_GetPCLK1Freq();
    // uint32_t prescalers[] = {
    //     SPI_BAUDRATEPRESCALER_2, SPI_BAUDRATEPRESCALER_4, SPI_BAUDRATEPRESCALER_8, SPI_BAUDRATEPRESCALER_16,
    //     SPI_BAUDRATEPRESCALER_32, SPI_BAUDRATEPRESCALER_64, SPI_BAUDRATEPRESCALER_128, SPI_BAUDRATEPRESCALER_256
    // };
    // int i = 0;
    // for (int j = 2; i < 7; ++i, j <<= 1) {
    //     if (EPD_SPI_CLOCK_FREQ >= freq / j)
    //         break;
    // }

    spi->Init.FirstBit          = SPI_FIRSTBIT_MSB;
    spi->Init.TIMode            = SPI_TIMODE_DISABLE;
    spi->Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
    spi->Init.CRCPolynomial     = 7;
    spi->Init.CRCLength         = SPI_CRC_LENGTH_DATASIZE;
    spi->Init.NSSPMode          = SPI_NSS_PULSE_DISABLE;

    // uint32_t pull = spi->Init.CLKPolarity == SPI_POLARITY_LOW ? GPIO_PULLDOWN : GPIO_PULLUP;

    if (spi->Instance == SPI1)
        __HAL_RCC_SPI1_CLK_ENABLE();

    if (spi->Instance == SPI3)
        __HAL_RCC_SPI3_CLK_ENABLE();

    HAL_SPI_Init(spi);
    // NOTE In order to set correctly the SPI polarity we need to enable the peripheral
    __HAL_SPI_ENABLE(spi);
}

void EPD_SPI_DeInit(EPD_SPI_Handle *spi)
{
    if (!spi)
        return;

    HAL_SPI_DeInit(spi);

    if (spi->Instance == SPI1) {
        __HAL_RCC_SPI1_FORCE_RESET();
        __HAL_RCC_SPI1_RELEASE_RESET();
        __HAL_RCC_SPI1_CLK_DISABLE();
    }

    if (spi->Instance == SPI3) {
        __HAL_RCC_SPI3_FORCE_RESET();
        __HAL_RCC_SPI3_RELEASE_RESET();
        __HAL_RCC_SPI3_CLK_DISABLE();
    }
}

void EPD_GPIO_Init(EPD *epd)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOB, EPD_SRCS_Pin|EPD_RST_Pin, GPIO_PIN_RESET);

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOA, EPD_DC_Pin|EPD_CS_Pin, GPIO_PIN_RESET);

    /*Configure GPIO pins : EPD_SRCS_Pin EPD_RST_Pin */
    GPIO_InitStruct.Pin = EPD_SRCS_Pin|EPD_RST_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /*Configure GPIO pins : EPD_DC_Pin EPD_CS_Pin */
    GPIO_InitStruct.Pin = EPD_DC_Pin|EPD_CS_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /*Configure GPIO pin : EPD_BUSY_Pin */
    GPIO_InitStruct.Pin = EPD_BUSY_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(EPD_BUSY_GPIO_Port, &GPIO_InitStruct);
}

void EPD_GPIO_DeInit(EPD *epd)
{
    HAL_GPIO_DeInit(GPIOA, EPD_DC_Pin | EPD_CS_Pin);
    HAL_GPIO_DeInit(GPIOB, EPD_SRCS_Pin | EPD_RST_Pin | EPD_BUSY_Pin);

    __HAL_RCC_GPIOA_CLK_DISABLE();
    __HAL_RCC_GPIOB_CLK_DISABLE();
}