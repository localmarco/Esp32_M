#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "driver/i2c.h"

#include "oled.h"

static const char *TAG = "[OLED]";

static unsigned char canvas[OLED_W * OLED_H / 8] = {0x00};
SemaphoreHandle_t r_mux = NULL;
SemaphoreHandle_t w_mux = NULL;

static esp_err_t i2c_master_init() {
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = OLED_I2C_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = OLED_I2C_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = OLED_I2C_SPEED;
    i2c_param_config(OLED_I2C_NUM, &conf);
    return i2c_driver_install( OLED_I2C_NUM, conf.mode, 0, 0, 0);
}

static esp_err_t OLED_i2c_write_cmd(i2c_port_t port, OLED_DATA_TYPE type, unsigned char data) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (OLED_I2C_ADDR << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, OLED_I2C_CMD, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, data, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(port, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
	return ret;
}

static esp_err_t OLED_i2c_write_data(i2c_port_t port, OLED_DATA_TYPE type, unsigned char *data, unsigned int len) {
    esp_err_t ret = ESP_OK;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (OLED_I2C_ADDR << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, OLED_I2C_DATA, ACK_CHECK_EN);
    for (int i = 0; i < len; i++)
        i2c_master_write_byte(cmd, data[i], ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(port, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
	return ret;
}

static void OLED_draw_frame(unsigned char *f) {
    unsigned char i;
    for(i = 0; i < (OLED_H / 8); i++) {
        OLED_i2c_write_cmd(OLED_I2C_NUM, OLED_CMD, 0xB0+i);
        OLED_i2c_write_cmd(OLED_I2C_NUM, OLED_CMD, 0x00);
        OLED_i2c_write_cmd(OLED_I2C_NUM, OLED_CMD, 0x10);
        OLED_i2c_write_data(OLED_I2C_NUM, OLED_DATA, f + (i * OLED_W), OLED_W);
    }
}

static void OLED_clear(unsigned char f) {
    unsigned char i, n;
    unsigned char buff[1];
    buff[0] = f;
    for(i = 0; i < (OLED_H / 8); i++) {
        OLED_i2c_write_cmd(OLED_I2C_NUM, OLED_CMD, 0xB0+i);
        OLED_i2c_write_cmd(OLED_I2C_NUM, OLED_CMD, 0x00);
        OLED_i2c_write_cmd(OLED_I2C_NUM, OLED_CMD, 0x10);
        for(n = 0; n < OLED_W; n++) {
            OLED_i2c_write_data(OLED_I2C_NUM, OLED_DATA, buff, SIZEOF(buff));
        }
    }
}

/* SSD1306 screen data
 * For page mode:
 * 128 * 32
 * [ 0 ... 127 ] -> [ 0 ]...[ 0 ]
 * [ 0 ... 127 ]    [ . ]...[ . ]
 * [ 0 ... 127 ]    [ . ]...[ . ]
 * [ 0 ... 127 ]    [ 7 ]...[ 7 ]
 * page(32) = 4 x 8
 * column = 128
 */
static void OLED_set_pos(unsigned int x, unsigned int y) {
	/* set page */
    OLED_i2c_write_cmd(OLED_I2C_NUM, OLED_CMD, 0xB0 + y);
	/* set column */
    OLED_i2c_write_cmd(OLED_I2C_NUM, OLED_CMD, ((x & 0xF0)>>4) | 0x10);
    OLED_i2c_write_cmd(OLED_I2C_NUM, OLED_CMD, ( x & 0x0F));
}

/* x0, y0: start 
 * x1, y1: end
 */
static void OLED_draw_pic(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char *data, unsigned int len) {
    unsigned char x, y;
    unsigned int i = 0;
    if (y1 % 8 == 0) {
        y = y1 / 8;
    } else {
        y = y1 / 8 + 1;
    }
    for ( y = y0; y < y1; y++) {
        OLED_set_pos(x0, y);
        for ( x = x0; x < x1; x++) {
            OLED_i2c_write_data(OLED_I2C_NUM, OLED_DATA, data + i, 1);
            i++;
        }
    }
}

void OLED_set_display(OLED_DISPLAY display) {
    OLED_i2c_write_cmd(OLED_I2C_NUM, OLED_CMD, 0x8D);
    switch (display)
    {
        case OLED_DISPLAY_ON:
            OLED_i2c_write_cmd(OLED_I2C_NUM, OLED_CMD, 0x14);
            OLED_i2c_write_cmd(OLED_I2C_NUM, OLED_CMD, 0xAF);
            break;
        case OLED_DISPLAY_OFF:
            OLED_i2c_write_cmd(OLED_I2C_NUM, OLED_CMD, 0x10);
            OLED_i2c_write_cmd(OLED_I2C_NUM, OLED_CMD, 0xAE);   
            break;
        default:
            break;
    }
}

void OLED_fill_surface(unsigned char* buff) {
    xSemaphoreTake(w_mux, portMAX_DELAY);
    MEMCPY(canvas, buff, OLED_W * OLED_H / 8);
    xSemaphoreGive(r_mux);
}

static void OLED_update_task(void *arg) {
    while (true) {
        if (xSemaphoreTake(r_mux, portMAX_DELAY) == pdTRUE ) {
			OLED_draw_frame(canvas);
        	xSemaphoreGive(w_mux);
        } else {
            ESP_LOGI(TAG, " --- [OLED_update_task] Get Semaphore Error---");
        }
    }
    vSemaphoreDelete(r_mux);
    vSemaphoreDelete(w_mux);
    vTaskDelete(NULL);
}

esp_err_t OLED_init() {
    ESP_LOGI(TAG, "---Init Oled---");
    esp_err_t ret = ESP_OK;
    ret += i2c_master_init();
	ret += OLED_i2c_write_cmd(OLED_I2C_NUM, OLED_CMD, 0xAE);
    ret += OLED_i2c_write_cmd(OLED_I2C_NUM, OLED_CMD, 0x40);
    ret += OLED_i2c_write_cmd(OLED_I2C_NUM, OLED_CMD, 0xB0);
    ret += OLED_i2c_write_cmd(OLED_I2C_NUM, OLED_CMD, 0xC8);
    ret += OLED_i2c_write_cmd(OLED_I2C_NUM, OLED_CMD, 0x81); //设置对比度(0x00~0xFF)
    ret += OLED_i2c_write_cmd(OLED_I2C_NUM, OLED_CMD, 0xFF);
    ret += OLED_i2c_write_cmd(OLED_I2C_NUM, OLED_CMD, 0xA1); //段重映射(A0h/A1h)
    ret += OLED_i2c_write_cmd(OLED_I2C_NUM, OLED_CMD, 0xA6); // 正常 (0xA6)/反转(0xA7)显示
    ret += OLED_i2c_write_cmd(OLED_I2C_NUM, OLED_CMD, 0xA8); //复用率(A8h) [16~63]
    ret += OLED_i2c_write_cmd(OLED_I2C_NUM, OLED_CMD, 0x1F);
    ret += OLED_i2c_write_cmd(OLED_I2C_NUM, OLED_CMD, 0xD3);
    ret += OLED_i2c_write_cmd(OLED_I2C_NUM, OLED_CMD, 0x00);
    ret += OLED_i2c_write_cmd(OLED_I2C_NUM, OLED_CMD, 0xD5); //DCLK
    ret += OLED_i2c_write_cmd(OLED_I2C_NUM, OLED_CMD, 0xF0);
    ret += OLED_i2c_write_cmd(OLED_I2C_NUM, OLED_CMD, 0xD9); //充电周期
    ret += OLED_i2c_write_cmd(OLED_I2C_NUM, OLED_CMD, 0x22);
    ret += OLED_i2c_write_cmd(OLED_I2C_NUM, OLED_CMD, 0xDA); //列引脚硬件配置
    ret += OLED_i2c_write_cmd(OLED_I2C_NUM, OLED_CMD, 0x02);
    ret += OLED_i2c_write_cmd(OLED_I2C_NUM, OLED_CMD, 0xDB); //调整VCOMH
    ret += OLED_i2c_write_cmd(OLED_I2C_NUM, OLED_CMD, 0x49);
    ret += OLED_i2c_write_cmd(OLED_I2C_NUM, OLED_CMD, 0x8D); // 打开显示
    ret += OLED_i2c_write_cmd(OLED_I2C_NUM, OLED_CMD, 0x14);
    ret += OLED_i2c_write_cmd(OLED_I2C_NUM, OLED_CMD, 0xAF);
	if (ESP_OK != ret) {
        ESP_LOGI(TAG, "--- OLED Init ret [%d] ---", ret);
		goto Err1;
	}
    OLED_clear(0x00);

    /* Init TaskWork. */
    r_mux = xSemaphoreCreateMutex();
    w_mux = xSemaphoreCreateMutex();
    if (r_mux == NULL || w_mux == NULL) {
		ESP_LOGI(TAG, " xSemaphoreCreateMutex error...");
		goto Err0;
    }
    xTaskCreate( OLED_update_task, "Oled_work_task", 1024 * 2,  NULL, 5, NULL);
    // vTaskStartScheduler();
    return ESP_OK;
Err0:
	if (r_mux) vSemaphoreDelete(r_mux);
	if (w_mux) vSemaphoreDelete(w_mux);
	return ESP_ERR_NO_MEM;
Err1:
	return ESP_FAIL;
}
