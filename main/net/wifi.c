#include "wifi.h"

#define SIZEOF(_x) (sizeof(_x)/sizeof(_x[0]))

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
    wifi_config_t sta_config = {
        .sta = {
            .ssid = STA_SSID,
            .password = STA_PASSWORD,
            .bssid_set = false
        }
    };
    wifi_config_t ap_config = {
        .ap = {
            .ssid = AP_SSID,
            .password = AP_PASSWORD,
            .ssid_len = 0,
            .max_connection = 1,
            .authmode = WIFI_AUTH_WPA2_PSK
        }
    };
    ESP_LOGI(TAG, "Connect SSID [%s][%s]", sta_config.sta.ssid, sta_config.sta.password);
    ESP_ERROR_CHECK( esp_wifi_set_config( WIFI_IF_STA, &sta_config) );
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

static esp_err_t http_get_plane_up(httpd_req_t *req) {
	ESP_LOGI(TAG, " Action Up");
    return ESP_OK;
}

static esp_err_t http_get_plane_down(httpd_req_t *req) {
	ESP_LOGI(TAG, " Action Down");
    return ESP_OK;
}

static esp_err_t http_get_plane_left(httpd_req_t *req) {
	ESP_LOGI(TAG, " Action Left");
    return ESP_OK;
}

static esp_err_t http_get_plane_right(httpd_req_t *req) {
	ESP_LOGI(TAG, " Action Right");
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
		.uri       = "/plane",
		.method    = HTTP_GET,
		.handler   = http_get_plane,
		.user_ctx  = NULL
	},
	{
		.uri       = "/plane/up",
		.method    = HTTP_GET,
		.handler   = http_get_plane_up,
		.user_ctx  = NULL
	},
	{
		.uri       = "/plane/down",
		.method    = HTTP_GET,
		.handler   = http_get_plane_down,
		.user_ctx  = NULL
	},
	{
		.uri       = "/plane/left",
		.method    = HTTP_GET,
		.handler   = http_get_plane_left,
		.user_ctx  = NULL
	},
	{
		.uri       = "/plane/right",
		.method    = HTTP_GET,
		.handler   = http_get_plane_right,
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
