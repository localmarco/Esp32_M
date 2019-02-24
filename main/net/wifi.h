#ifndef __WIFI_H
#define __WIFI_H

#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_http_server.h"

#include "nvs.h"
#include "game.h"

#define STA_SSID CONFIG_APSTA_STA_SSID
#define STA_PASSWORD CONFIG_APSTA_STA_PASSWORD
#define AP_SSID CONFIG_APSTA_AP_SSID
#define AP_PASSWORD CONFIG_APSTA_AP_PASSWORD

/* init wifi and ap */
esp_err_t event_handler(void *ctx, system_event_t *event);
esp_err_t init_esp_wifi();
/* http server */
esp_err_t start_http_server();
#endif
