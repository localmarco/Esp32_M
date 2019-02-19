#ifndef __OLED_H
#define __OLED_H
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "driver/i2c.h"

#define SIZEOF(x) (sizeof(x)/sizeof(x[0])) 

#define OLED_I2C_SDA_IO CONFIG_OLED_I2C_SDA
#define OLED_I2C_SCL_IO CONFIG_OLED_I2C_SCL

#define OLED_I2C_ADDR 0x3C
#define OLED_I2C_CMD 0x00
#define OLED_I2C_DATA 0x40
#define OLED_I2C_NUM I2C_NUM_0
#define OLED_I2C_SPEED (100 * 1000)  // 100kbit/s
#define ACK_CHECK_EN 0x01

#define OLED_W 128
#define OLED_H 32

typedef enum {
    OLED_CMD = 0,
    OLED_DATA
} OLED_DATA_TYPE;

typedef enum {
    OLED_DISPLAY_OFF = 0,
    OLED_DISPLAY_ON
} OLED_DISPLAY;

error_t OLED_init();
void OLED_set_display(OLED_DISPLAY display);
void OLED_fill_surface(unsigned char* buff);
#endif
