#ifndef __NVS_H_
#define __NVS_H_
#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"

#define LOG(fmt, ...) ESP_LOGI("[NVS]", fmt, ##__VA_ARGS__)

esp_err_t nvs_get_str_by_key(char* key, char *str, int size);
esp_err_t nvs_set_str_by_key(char* key, char *str);
#endif
