#include "wifi.h"

#define SIZEOF(_x) (sizeof(_x)/sizeof(_x[0]))

#define NVS_STA_SSID_KEY "stassid"
#define NVS_STA_PASSWD_KEY "stapasswd"
#define NVS_AP_SSID_KEY "apssid"
#define NVS_AP_PASSWD_KEY "appasswd"

static const char *TAG = "[Wifi]";

esp_err_t event_handler(void *ctx, system_event_t *event) {
    ESP_LOGI(TAG, "Handler Event [%d]", event->event_id);
    switch ( event->event_id)
    {
        case SYSTEM_EVENT_STA_START:
        case SYSTEM_EVENT_STA_DISCONNECTED:
            ESP_LOGI(TAG, " Sta connect ...");
            ESP_ERROR_CHECK(esp_wifi_connect());
            break;
		case SYSTEM_EVENT_STA_GOT_IP:
            ESP_LOGI(TAG, " Sta Got IP: %s", 
					ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
		case SYSTEM_EVENT_AP_STACONNECTED:
			ESP_LOGI(TAG, "station:"MACSTR" join, AID=%d",
					MAC2STR(event->event_info.sta_connected.mac),
					event->event_info.sta_connected.aid);
			break;
		case SYSTEM_EVENT_AP_STADISCONNECTED:
			ESP_LOGI(TAG, "station:"MACSTR"leave, AID=%d",
					MAC2STR(event->event_info.sta_disconnected.mac),
					event->event_info.sta_disconnected.aid);
			break;
		case SYSTEM_EVENT_AP_STAIPASSIGNED:
            ESP_LOGI(TAG, " Ap SYSTEM_EVENT_AP_STAIPASSIGNED...");
			break;
        case SYSTEM_EVENT_AP_START:
            ESP_LOGI(TAG, " Ap Start...");
            break;
        case SYSTEM_EVENT_AP_STOP:
            ESP_LOGI(TAG, " Ap Stop...");
            break;
        default:
            break;
    }
    return ESP_OK;
}

esp_err_t init_esp_wifi() {
    tcpip_adapter_init();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_APSTA) );
	char ssid[32] = {0x00};
	char passwd[64] = {0x00};
    wifi_config_t sta_config = {
        .sta = {
            .ssid = STA_SSID,
            .password = STA_PASSWORD,
            .bssid_set = false
        }
    };
	/* Get ssid by nvs */
	esp_err_t err = nvs_get_str_by_key(NVS_STA_SSID_KEY, ssid, 32);
	if (ESP_OK == err) {
		memset(sta_config.sta.ssid, 0x00, 32);
		memcpy(sta_config.sta.ssid, ssid, strlen(ssid));
	}
	err = nvs_get_str_by_key(NVS_STA_PASSWD_KEY, passwd, 64);
	if (ESP_OK == err) {
		memset(sta_config.sta.password, 0x00, 64);
		memcpy(sta_config.sta.password, passwd, strlen(passwd));
	}
    ESP_LOGI(TAG, "Connect STA Info [%s][%s]", sta_config.sta.ssid, sta_config.sta.password);
    ESP_ERROR_CHECK( esp_wifi_set_config( WIFI_IF_STA, &sta_config) );
    wifi_config_t ap_config = {
        .ap = {
            .ssid = AP_SSID,
            .password = AP_PASSWORD,
            .ssid_len = 0,
            .max_connection = 2,
            .authmode = WIFI_AUTH_WPA2_PSK
        }
    };
	err = nvs_get_str_by_key(NVS_AP_SSID_KEY, ssid, 32);
	if (ESP_OK == err) {
		memset(ap_config.sta.ssid, 0x00, 32);
		memcpy(ap_config.sta.ssid, ssid, strlen(ssid));
	}
	err = nvs_get_str_by_key(NVS_AP_PASSWD_KEY, passwd, 64);
	if (ESP_OK == err) {
		memset(ap_config.sta.password, 0x00, 64);
		memcpy(ap_config.sta.password, passwd, strlen(passwd));
	}
    ESP_LOGI(TAG, "Connect AP Info [%s][%s]", ap_config.ap.ssid, ap_config.sta.password);
    ESP_ERROR_CHECK( esp_wifi_set_config( WIFI_IF_AP, &ap_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
    ESP_ERROR_CHECK( esp_wifi_connect() );
    return ESP_OK;
}

static esp_err_t http_get_index(httpd_req_t *req) {
    extern const unsigned char index_html_start[] asm("_binary_index_html_start");
    extern const unsigned char index_html_end[]   asm("_binary_index_html_end");
    const size_t index_html_file_size = (index_html_end - index_html_start);
    httpd_resp_set_type(req, "text");
    httpd_resp_send(req, (const char *)index_html_start, index_html_file_size);
    return ESP_OK;
}

static esp_err_t http_get_plane(httpd_req_t *req) {
    extern const unsigned char plane_html_start[] asm("_binary_plane_html_start");
    extern const unsigned char plane_html_end[]   asm("_binary_plane_html_end");
    const size_t plane_html_file_size = (plane_html_end - plane_html_start);
    httpd_resp_set_type(req, "text");
    httpd_resp_send(req, (const char *)plane_html_start, plane_html_file_size);
    return ESP_OK;
}

static esp_err_t http_plane_action(httpd_req_t *req) {
	ESP_LOGI(TAG, " http post wifi");
    char buf[100] = {0x00};
    int ret, remaining = req->content_len;
    while (remaining > 0) {
        /* Read the data for the request */
        if ((ret = httpd_req_recv(req, buf,
                        MIN(remaining, sizeof(buf)))) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                /* Retry receiving if timeout occurred */
                continue;
            }
            return ESP_FAIL;
        }
        remaining -= ret;
        /* Log data received */
        ESP_LOGI(TAG, "Receive: %.*s", ret, buf);
    }
	if (0 == strncmp(buf, "up", strlen("up"))) {
		update_hero(C_UP);
	}else if (0 == strncmp(buf, "left", strlen("left"))) {
		update_hero(C_LEFT);
	}else if (0 == strncmp(buf, "right", strlen("right"))) {
		update_hero(C_RIGHT);
	}else if (0 == strncmp(buf, "down", strlen("down"))) {
		update_hero(C_DOWN);
	}else if (0 == strncmp(buf, "shoot", strlen("shoot"))) {
		update_hero(C_SHOOT);
	}

    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static esp_err_t http_post_wifi(httpd_req_t *req) {
	ESP_LOGI(TAG, " http post wifi");
    char buf[100] = {0x00};
    int ret, remaining = req->content_len;

    while (remaining > 0) {
        /* Read the data for the request */
        if ((ret = httpd_req_recv(req, buf,
                        MIN(remaining, sizeof(buf)))) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                /* Retry receiving if timeout occurred */
                continue;
            }
            return ESP_FAIL;
        }
        remaining -= ret;
        /* Log data received */
        ESP_LOGI(TAG, "Receive: %.*s", ret, buf);
    }
	ESP_LOGI(TAG, "Buff len %d", strlen(buf));
	if (strlen(buf) > 0) {
		char *r1,*r2;
		r1 = strtok(buf, "&");
		r2 = strtok(NULL, "&");
		if (strncmp(strtok(r1, "="), "ssid", strlen("ssid")) == 0) {
			nvs_set_str_by_key(NVS_STA_SSID_KEY, strtok(NULL, "="));
		}
		if (strncmp(strtok(r2, "="), "passwd", strlen("passwd")) == 0) {
			nvs_set_str_by_key(NVS_STA_PASSWD_KEY, strtok(NULL, "="));
		}
	}
    // End response
    //httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

httpd_uri_t basic_handlers[] = {
	{
		.uri       = "/",
		.method    = HTTP_GET,
		.handler   = http_get_index,
		.user_ctx  = NULL 
	},
	{
		.uri       = "/wifi",
		.method    = HTTP_POST,
		.handler   = http_post_wifi,
		.user_ctx  = NULL
	},
	{
		.uri       = "/plane",
		.method    = HTTP_GET,
		.handler   = http_get_plane,
		.user_ctx  = NULL
	},
	{
		.uri       = "/plane/action",
		.method    = HTTP_PUT,
		.handler   = http_plane_action,
		.user_ctx  = NULL
	}
};


esp_err_t start_http_server() {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;
    ESP_LOGI(TAG, "Starting HTTP Server");
    if (httpd_start(&server, &config) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start file server!");
        return ESP_FAIL;
    }
    for (int i = 0; i < sizeof(basic_handlers)/sizeof(httpd_uri_t); i++) {
        if (ESP_OK == httpd_register_uri_handler(server, &basic_handlers[i])) {
            ESP_LOGI(TAG, " HTTP Server register uri:%s", basic_handlers[i].uri);
        }
    }
	return ESP_OK;
}
