#ifndef HW_H
#define HW_H


//---------- LCD ----------//
#define HW_LCD_MISO  -1
#define HW_LCD_MOSI  11
#define HW_LCD_SCLK  12
#define HW_LCD_CS    10
#define HW_LCD_DC    9
#define HW_LCD_RST   -1
#define HW_LCD_BL    14

// esp-idf/components/driver/spi/include/driver/spi_master.h (v5.2, Line:29)
// #define SPI_MASTER_FREQ_40M (80 * 1000 * 1000 / 2) ///< 40MHz

#define HW_LCD_SPI_HOST SPI2_HOST
#define HW_LCD_SPI_FREQ SPI_MASTER_FREQ_40M // MHz

#define HW_LCD_INV 1
#define HW_LCD_DIR 0

#define HW_LCD_W 320
#define HW_LCD_H 240
#define HW_LCD_OFFSETX 0
#define HW_LCD_OFFSETY 0

#define HW_LCD_DRIVER 0

// esp-idf/components/driver/sdmmc/include/driver/sdmmc_types.h (v5.2, Line:181)
// #define SDMMC_FREQ_DEFAULT 20000 /*!< SD/MMC Default speed (limited by clock divider) */

#define HW_SD_SPI_HOST SPI2_HOST
#define HW_SD_SPI_FREQ SDMMC_FREQ_DEFAULT

//---------- RELAYS ----------//
#define HW_RELAY_1 15
#define HW_RELAY_2 16
#define HW_RELAY_3 17

//---------- ROTARY ENCODER ----------//
#define HW_ROTARY_ENC_A 1
#define HW_ROTARY_ENC_B 2
#define HW_ROTARY_ENC_BTN 3

//---------- THERMOMETERS ----------//
#define HW_ADC_AMBIENT 4
#define HW_ADC_MEAT 5

//---------- ULTRASONIC SENSOR ----------//
#define HW_ULTRASONIC_TRIG 6
#define HW_ULTRASONIC_ECHO 7


#endif