#include <stdio.h>
#include <esp_event_loop.h>
#include <esp_log.h>
#include <esp_system.h>
#include <driver/spi_master.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

#include "wifi.h"
#include "webui.h"
#include "ws.h"
#include "can.h"
#include "settings.h"
#include "lcdui.h"


static int must_cleanup = 0;
static EventGroupHandle_t my_event_group;
static const int WIFI_STA_CONNECTED = BIT0;
static const int WIFI_AP_REQUESTED = BIT1;
static const int WIFI_AP_STARTED = BIT2;

static const char *TAG = "MAIN";

static esp_err_t event_handler(void *ctx, system_event_t *event) {
	wifi_event(event);

	switch(event->event_id) {
	case SYSTEM_EVENT_STA_GOT_IP:
		must_cleanup = 1;
		break;
	case SYSTEM_EVENT_STA_DISCONNECTED:
		if (must_cleanup) {
			must_cleanup = 0;
		}
		break;
	default:
		break;
	}

	return ESP_OK;
}

static void IRAM_ATTR ap_mode_button_isr_handler(void* arg) {
	EventBits_t bits = xEventGroupGetBitsFromISR(my_event_group);
	if (bits & WIFI_AP_STARTED) {
		return;
	}

	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	BaseType_t xResult = xEventGroupSetBitsFromISR(
		my_event_group,
		WIFI_AP_REQUESTED,
		&xHigherPriorityTaskWoken
		);

	if (xResult == pdPASS) {
		if (xHigherPriorityTaskWoken == pdTRUE) {
			portYIELD_FROM_ISR();
		}
	}
}

void app_main() {
	my_event_group = xEventGroupCreate();

	tcpip_adapter_init();
	ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

	settings_init();
	cannelloni_init();
	wifi_start();

	webui_start();
	ws_start();
	cannelloni_start();

	spi_bus_config_t bus_config = {
		.mosi_io_num = GPIO_NUM_23,
		.miso_io_num = GPIO_NUM_19,
		.sclk_io_num = GPIO_NUM_18,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1,
	};
	ESP_ERROR_CHECK(spi_bus_initialize(VSPI_HOST, &bus_config, 1));

	lcdui_start();

	//Initialize GPIO interrupt to switch to AP mode
	gpio_config_t io_conf;
	io_conf.intr_type = GPIO_INTR_NEGEDGE;
	io_conf.pin_bit_mask = BIT(CONFIG_AP_BUTTON_GPIO);
	io_conf.mode = GPIO_MODE_INPUT;
	io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
	gpio_config(&io_conf);
	gpio_install_isr_service(0);
	gpio_isr_handler_add(GPIO_NUM_0, ap_mode_button_isr_handler, (void*) 0);

	while (1) {
		EventBits_t bits;
		const TickType_t xTicksToWait = 100 / portTICK_PERIOD_MS;
		bits = xEventGroupWaitBits(
			my_event_group,
			WIFI_AP_REQUESTED,
			pdTRUE,
			pdFALSE,
			xTicksToWait
			);

		if (bits & WIFI_AP_REQUESTED) {
			if (0 == (bits & WIFI_AP_STARTED)) {
				ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
				wifi_init_softap();
				xEventGroupSetBits(my_event_group, WIFI_AP_STARTED);
			}
		}

	}
}
