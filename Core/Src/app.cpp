#include "app.h"
#include "VL53L1X_api.h"
// #include "adafruit_154_mono_d27.h"
// #include "qr.h"
#include "stm32l4xx_hal.h"
// #include "main.h"

constexpr uint16_t vl53lx1_dev = 0x52;

// extern SPI_HandleTypeDef hspi3;

// using QR_Type = QR<2>;
// constexpr ECC ecc = ECC::L;
// constexpr int pix = 6;
// constexpr int pix_side = QR_Type::SIDE * pix;
// constexpr int start_x = 200 / 2 - pix_side / 2;
// constexpr int start_y = 200 / 2 - pix_side / 2;
// constexpr const char *str = "HELLO WORLD";

// static QR_Type qr;

// static EPD epd {
//     .cs     = { EPD_CS_GPIO_Port,   EPD_CS_Pin      },
//     .dc     = { EPD_DC_GPIO_Port,   EPD_DC_Pin      },
//     .srcs   = { EPD_SRCS_GPIO_Port, EPD_SRCS_Pin    },
//     .sdcs   = { NULL, 0 },
//     .rst    = { EPD_RST_GPIO_Port,  EPD_RST_Pin     },
//     .busy   = { EPD_BUSY_GPIO_Port, EPD_BUSY_Pin    },
//     .ena    = { NULL, 0 },
// };

// static Adafruit_154_Mono_D27 display(&epd, &hspi3);

bool APP_VL53L1_Init()
{
    int8_t status = 0;
    uint8_t sensor_state = 0;
    uint8_t byte = 0;
    uint16_t word = 0;
    // uint16_t xtalk;
    // int16_t offset;

    // Those basic I2C read functions can be used to check your own I2C functions
    status += VL53L1_RdByte(vl53lx1_dev, 0x010F, &byte); printf("VL53L1X Model_ID: %X\n", byte);
    status += VL53L1_RdByte(vl53lx1_dev, 0x0110, &byte); printf("VL53L1X Module_Type: %X\n", byte);
    status += VL53L1_RdWord(vl53lx1_dev, 0x010F, &word); printf("VL53L1X: %X\n", word);

    if (status != 0)
        return false;

    while (sensor_state == 0) {
        status = VL53L1X_BootState(vl53lx1_dev, &sensor_state);
        HAL_Delay(2);
    }
    printf("VL53L1X booted\n");

    // This function must to be called to initialize the sensor with the default setting 
    status = VL53L1X_SensorInit(vl53lx1_dev);

    // Optional functions to be used to change the main ranging parameters according the application requirements to get the best ranging performances
    status += VL53L1X_SetDistanceMode(vl53lx1_dev, 2);            // 1 = short, 2 = long
    status += VL53L1X_SetTimingBudgetInMs(vl53lx1_dev, 100);      // in ms possible values [20, 50, 100, 200, 500] 
    status += VL53L1X_SetInterMeasurementInMs(vl53lx1_dev, 100);  // in ms, IM must be > = TB 
    // status += VL53L1X_SetOffset(vl53lx1_dev, 20);                  // offset compensation in mm
    // status += VL53L1X_SetROI(vl53lx1_dev, 16, 16);                 // minimum ROI 4,4
    // status += VL53L1X_CalibrateOffset(vl53lx1_dev, 140, &offset);  // may take few second to perform the offset cal
    // status += VL53L1X_CalibrateXtalk(vl53lx1_dev, 1000, &xtalk);   // may take few second to perform the xtalk cal

    if (status != 0)
        return false;

    printf("VL53L1X Ultra Lite Driver Example running ...\n");

    // This function has to be called to enable the ranging
    if (VL53L1X_StartRanging(vl53lx1_dev))
        return false;
    return true;
}

bool APP_VL53L1_Measure()
{
    int8_t status = 0;
    uint8_t data_ready = 0;

    uint16_t distance, signal_rate, ambient_rate, spad_num;
    uint8_t range_status;

    while (data_ready == 0) {
        status = VL53L1X_CheckForDataReady(vl53lx1_dev, &data_ready);
        HAL_Delay(2);
    }
    status += VL53L1X_GetRangeStatus(vl53lx1_dev, &range_status);
    status += VL53L1X_GetDistance(vl53lx1_dev, &distance);
    status += VL53L1X_GetSignalRate(vl53lx1_dev, &signal_rate);
    status += VL53L1X_GetAmbientRate(vl53lx1_dev, &ambient_rate);
    status += VL53L1X_GetSpadNb(vl53lx1_dev, &spad_num);

    // NOTE Clear interrupt has to be called to enable next interrupt
    status += VL53L1X_ClearInterrupt(vl53lx1_dev);

    if (status != 0)
        return false;

    printf("VL53L1X DATA: \n\
            status:       0x%02x\n\
            range_status: %u\n\
            distance:     %u\n\
            signal_rate:  %u\n\
            ambient_rate: %u\n\
            spad_num:     %u\n\n", 
            status, range_status, distance, signal_rate, ambient_rate, spad_num);

    return true;
}

bool APP_EPD_Init()
{
    // qr.encode(str, strlen(str), ecc, -1);

    // display.begin();
    // display.clear_buffer();
    // display.display();
}

bool APP_EPD_Display()
{
    // display.clear_buffer();
    // for (int y = 0; y < QR_Type::SIDE; ++y) {
    //     for (int x = 0; x < QR_Type::SIDE; ++x) {
    //         display.fill_rect(
    //             start_x + x * pix, 
    //             start_y + y * pix, 
    //             pix, pix, 
    //             qr.module(x, y));
    //     }
    // }
    // display.display();
}