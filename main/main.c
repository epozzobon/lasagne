#include <stdio.h>
#include <esp_event_loop.h>
#include <esp_log.h>
#include <esp_system.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

#include "wifi.h"
#include "webui.h"
#include "ws.h"
#include "cannelloni.h"
#include "settings.h"

static int must_cleanup = 0;

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    wifi_event(event);

    switch(event->event_id) {
    case SYSTEM_EVENT_STA_GOT_IP:
        webui_start();
        ws_start();
        must_cleanup = 1;
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
    	if (must_cleanup) {
    		must_cleanup = 0;
			webui_stop();
	        ws_stop();
    	}
    	break;
    default:
        break;
    }

    return ESP_OK;
}

void app_main()
{
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

    settings_init();
	cannelloni_init();
    wifi_start();

    cannelloni_start();
}
