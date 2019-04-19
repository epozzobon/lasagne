#include <string.h>
#include <esp_wifi.h>
#include <esp_event_loop.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include <lwip/err.h>
#include <lwip/sys.h>

#include "settings.h"

#define DEFAULT_WIFI_SSID      CONFIG_DEFAULT_WIFI_STA_SSID
#define DEFAULT_WIFI_PASS      CONFIG_DEFAULT_WIFI_STA_PASS
#define DEFAULT_WIFI_AP_SSID      CONFIG_DEFAULT_WIFI_AP_SSID
#define DEFAULT_WIFI_AP_PASS      CONFIG_DEFAULT_WIFI_AP_PASS

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about one event
 * - are we connected to the AP with an IP? */
const int WIFI_CONNECTED_BIT = BIT0;

static const char *TAG = "wifi station";

static int s_retry_num = 0;

void wifi_init_softap() {
	wifi_config_t wifi_config = {
		.ap = {
			.ssid = DEFAULT_WIFI_AP_SSID,
			.ssid_len = strlen(DEFAULT_WIFI_AP_SSID),
			.password = DEFAULT_WIFI_AP_PASS,
			.max_connection = CONFIG_MAX_STA_CONN,
			.authmode = WIFI_AUTH_WPA_WPA2_PSK
		},
	};

	settings_read("wifi_ap_ssid",
	              wifi_config.ap.ssid,
	              sizeof(wifi_config.sta.ssid));
	settings_read("wifi_ap_password",
	              wifi_config.ap.password,
	              sizeof(wifi_config.sta.password));
	wifi_config.ap.ssid_len = strlen((char *) wifi_config.ap.ssid);
	if (0 == strlen((char *) wifi_config.ap.password)) {
		wifi_config.ap.authmode = WIFI_AUTH_OPEN;
	}

	ESP_LOGI(TAG, "wifi_init_softap finished.SSID:%s password:%s",
	         wifi_config.ap.ssid, wifi_config.ap.password);


	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
	ESP_ERROR_CHECK(esp_wifi_start());

	ESP_LOGI(TAG, "wifi_init_softap finished.SSID:%s password:%s",
	         wifi_config.ap.ssid, wifi_config.ap.password);
}

void wifi_init_sta() {
	s_wifi_event_group = xEventGroupCreate();

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_LOGI(TAG, "esp_wifi_init");
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	wifi_config_t wifi_config = { };

	settings_read("wifi_ssid", wifi_config.sta.ssid, sizeof(wifi_config.sta.ssid));
	settings_read("wifi_password", wifi_config.sta.password, sizeof(wifi_config.sta.password));

	ESP_LOGI(TAG, "esp_wifi_set_mode");
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
	ESP_LOGI(TAG, "esp_wifi_set_config");
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
	ESP_LOGI(TAG, "esp_wifi_start");
	ESP_ERROR_CHECK(esp_wifi_start() );

	ESP_LOGI(TAG, "wifi_init_sta finished.");
	ESP_LOGI(TAG, "connect to ap SSID:%s password:%s",
	         wifi_config.sta.ssid, wifi_config.sta.password);
}

void wifi_settings_listener(const char *key, const uint8_t *value, size_t size, void *closure) {
	ESP_LOGI(TAG, "wifi settings were changed.");
}

void wifi_settings_init() {
	// Initialize the wifi settings
	uint8_t tmp[64];
	wifi_config_t wifi_config;

	strcpy((char*) tmp, DEFAULT_WIFI_SSID);
	settings_register("wifi_ssid", tmp, sizeof(wifi_config.sta.ssid));
	settings_subscribe("wifi_ssid", wifi_settings_listener, NULL);

	strcpy((char*) tmp, DEFAULT_WIFI_AP_SSID);
	settings_register("wifi_ap_ssid", tmp, sizeof(wifi_config.ap.ssid));
	settings_subscribe("wifi_ap_ssid", wifi_settings_listener, NULL);

	strcpy((char*) tmp, DEFAULT_WIFI_PASS);
	settings_register("wifi_password", tmp, sizeof(wifi_config.sta.password));
	settings_subscribe("wifi_password", wifi_settings_listener, NULL);

	strcpy((char*) tmp, DEFAULT_WIFI_AP_PASS);
	settings_register("wifi_ap_password", tmp, sizeof(wifi_config.ap.password));
	settings_subscribe("wifi_ap_password", wifi_settings_listener, NULL);
}

void wifi_start() {
	// Initialize the wifi settings
	wifi_settings_init();

	// Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);


	s_wifi_event_group = xEventGroupCreate();

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
	wifi_init_sta();

}

esp_err_t wifi_event(system_event_t *event) {
	switch(event->event_id) {
	case SYSTEM_EVENT_STA_START:
	{
		esp_wifi_connect();
		break;
	}
	case SYSTEM_EVENT_STA_GOT_IP:
	{
		ESP_LOGI(TAG, "got ip:%s",
		         ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
		s_retry_num = 0;
		xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
		break;
	}
	case SYSTEM_EVENT_STA_DISCONNECTED:
	{
		esp_wifi_connect();
		xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
		s_retry_num++;
		ESP_LOGW(TAG,"retry to connect to the AP");
		break;
	}
	case SYSTEM_EVENT_AP_STACONNECTED:
	{
		ESP_LOGI(TAG, "station:"MACSTR " join, AID=%d",
		         MAC2STR(event->event_info.sta_connected.mac),
		         event->event_info.sta_connected.aid);
		break;
	}
	case SYSTEM_EVENT_AP_STADISCONNECTED:
	{
		ESP_LOGI(TAG, "station:"MACSTR "leave, AID=%d",
		         MAC2STR(event->event_info.sta_disconnected.mac),
		         event->event_info.sta_disconnected.aid);
		break;
	}

	default:
		break;
	}
	return ESP_OK;
}

