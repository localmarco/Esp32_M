#include "nvs.h"

/* Define this name in partitions.csv */
#define NVS_STORAGE_NAME "storage"

esp_err_t nvs_get_str_by_key(char *key, char *str, int size) {
	nvs_handle my_handle = NULL;
	esp_err_t err = nvs_open(NVS_STORAGE_NAME, NVS_READWRITE, &my_handle);
	if (err != ESP_OK) {
		LOG("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
	} else {
        unsigned int len;
		err = nvs_get_str(my_handle, key, NULL, &len);
		LOG("Get key [%s] len[%d] err [%d]", key, len, err);
		if ( ESP_OK == err) {
			switch (err) {
				case ESP_OK:
					LOG("NVS Get Key [%s] done.len [%d]\n", key, len);
					if (len > size) {
						len = size - 1;
					} 
					if ( (err = nvs_get_str(my_handle, key, str, &len)) == ESP_OK) {
						printf("String associated with key '%s' is %s \n", key, str);
					}
					break;
				case ESP_ERR_NVS_NOT_FOUND:
					LOG("NVS Storage Key[%s] is not found...\n", key);
					break;
				default :
					LOG("Error (%s) reading!\n", esp_err_to_name(err));
			}
		}
		nvs_close(my_handle);
	}
	return err;
}

esp_err_t nvs_set_str_by_key(char *key, char *str) {
	nvs_handle my_handle = NULL;
	esp_err_t err = nvs_open(NVS_STORAGE_NAME, NVS_READWRITE, &my_handle);
	if (err != ESP_OK) {
		LOG("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
	} else {
        err = nvs_set_str(my_handle, key, str);
        err = nvs_commit(my_handle);
        LOG("NVS set [%s][%s] [%s]", key, str, (err != ESP_OK) ? "Failed!" : "Done");
		nvs_close(my_handle);
	}
	return err;
}
