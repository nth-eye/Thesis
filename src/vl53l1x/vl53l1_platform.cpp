
/* 
* This file is part of VL53L1 Platform 
* 
* Copyright (c) 2016, STMicroelectronics - All Rights Reserved 
* 
* License terms: BSD 3-clause "New" or "Revised" License. 
* 
* Redistribution and use in source and binary forms, with or without 
* modification, are permitted provided that the following conditions are met: 
* 
* 1. Redistributions of source code must retain the above copyright notice, this 
* list of conditions and the following disclaimer. 
* 
* 2. Redistributions in binary form must reproduce the above copyright notice, 
* this list of conditions and the following disclaimer in the documentation 
* and/or other materials provided with the distribution. 
* 
* 3. Neither the name of the copyright holder nor the names of its contributors 
* may be used to endorse or promote products derived from this software 
* without specific prior written permission. 
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE 
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
* 
*/

#include "vl53l1_platform.h"
#include "stm32l4xx_hal.h"

#define TX_TIMEOUT_MS   100
#define RX_TIMEOUT_MS   100

extern I2C_HandleTypeDef hi2c1;
static I2C_HandleTypeDef *i2c = &hi2c1;
static HAL_StatusTypeDef res;

static HAL_StatusTypeDef VL53L1_SendIndex(uint16_t dev, uint16_t index)
{
    uint8_t idx_bytes[] = { index >> 8, index & 0xff };

    return HAL_I2C_Master_Transmit(i2c, dev, idx_bytes, 2, TX_TIMEOUT_MS);
}

int8_t VL53L1_WriteMulti(uint16_t dev, uint16_t index, uint8_t *pdata, uint32_t count) 
{
    res = VL53L1_SendIndex(dev, index);

    if (res != HAL_OK)
        return res;

    return HAL_I2C_Master_Transmit(i2c, dev, pdata, count, TX_TIMEOUT_MS);
}

int8_t VL53L1_ReadMulti(uint16_t dev, uint16_t index, uint8_t *pdata, uint32_t count)
{
    res = VL53L1_SendIndex(dev, index);

    if (res != HAL_OK)
        return res;

    return HAL_I2C_Master_Receive(i2c, dev, pdata, count, RX_TIMEOUT_MS);
}

int8_t VL53L1_WrByte(uint16_t dev, uint16_t index, uint8_t data) 
{
    uint8_t to_send[] = { index >> 8, index & 0xff, data };

    return HAL_I2C_Master_Transmit(i2c, dev, to_send, 3, TX_TIMEOUT_MS);
}

int8_t VL53L1_WrWord(uint16_t dev, uint16_t index, uint16_t data) 
{
    uint8_t to_send[] = { index >> 8, index & 0xff, data >> 8, data & 0xff };

    return HAL_I2C_Master_Transmit(i2c, dev, to_send, 4, TX_TIMEOUT_MS);
}

int8_t VL53L1_WrDWord(uint16_t dev, uint16_t index, uint32_t data) 
{
    uint8_t to_send[] = 
    { 
        index >> 8, 
        index &  0xff, 
        data  >> 24, 
        data  >> 16,
        data  >> 8, 
        data  &  0xff 
    };
    return HAL_I2C_Master_Transmit(i2c, dev, to_send, 6, TX_TIMEOUT_MS);
}

int8_t VL53L1_RdByte(uint16_t dev, uint16_t index, uint8_t *data) 
{
    res = VL53L1_SendIndex(dev, index);

    if (res != HAL_OK)
        return res;

    return HAL_I2C_Master_Receive(i2c, dev, data, 1, RX_TIMEOUT_MS);
}

int8_t VL53L1_RdWord(uint16_t dev, uint16_t index, uint16_t *data) 
{
    res = VL53L1_SendIndex(dev, index);

    if (res != HAL_OK)
        return res;

    uint8_t to_receive[2];

    res = HAL_I2C_Master_Receive(i2c, dev, to_receive, 2, RX_TIMEOUT_MS);
    
    *data = (uint16_t) (to_receive[0] << 8) | (uint16_t) to_receive[1];
        
    return res;
}

int8_t VL53L1_RdDWord(uint16_t dev, uint16_t index, uint32_t *data) 
{
    res = VL53L1_SendIndex(dev, index);

    if (res != HAL_OK)
        return res;

    uint8_t to_receive[4];

    res = HAL_I2C_Master_Receive(i2c, dev, to_receive, 4, RX_TIMEOUT_MS);
    
    *data = (uint32_t) (to_receive[0] << 24) | 
            (uint32_t) (to_receive[1] << 16) |
            (uint32_t) (to_receive[2] << 16) |
            (uint32_t) (to_receive[3]      );
        
    return res;
}

int8_t VL53L1_WaitMs(uint16_t dev, int32_t wait_ms)
{
    HAL_Delay(wait_ms);
    return 0;
}
