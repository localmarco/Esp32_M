#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include "driver/i2c.h"

#include "oled/oled.h"
#include "net/wifi.h"
#include "game/game.h"

#define TAG "[MAIN]"

void app_main(void) {
	esp_err_t ret = ESP_OK;
	ret = nvs_flash_init();
    ESP_ERROR_CHECK( ret );
    
    /* Init Wifi/AP */
    ret = init_esp_wifi();
    ESP_ERROR_CHECK( ret );

    /* Start Http server */
	ret = start_http_server();
    ESP_ERROR_CHECK( ret );

    /* Init OLED Display. */
    ret = OLED_init();
    /* Game */
    if (ESP_OK == ret)
        GameStart();
}
