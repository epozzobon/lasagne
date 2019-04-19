#include <esp_log.h>
#include <esp_system.h>
#include <sys/param.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <sdkconfig.h>
#include <driver/gpio.h>

#include "lcdui.h"

#ifdef CONFIG_LCDUI_ENABLED
#include "tft.h"
#include "font.h"

static const char *TAG = "LCDUI";
static ili_device_handle_t ili_dev = NULL;

static esp_err_t lcdui_print_string(const char *string, uint16_t x, uint16_t y);

static void lcdui_main(void *params) {
	esp_err_t r;
	ili_config_t ili_config = {
		.bckl_io_num = GPIO_NUM_14,
		.dc_io_num = GPIO_NUM_21,
		.spics_io_num = GPIO_NUM_5
	};

	ESP_ERROR_CHECK(ili_bus_add_device(VSPI_HOST, &ili_config, &ili_dev));

	const TickType_t xTicksToWait = 1000 / portTICK_PERIOD_MS;

	ili_init(ili_dev);

	// Setup vertical scrolling area
	ESP_ERROR_CHECK(lcdui_print_string("0123456789ABCDEF", 16, 8));
	vTaskDelay(xTicksToWait);

	while (1) {
		vTaskDelay(xTicksToWait);
		r = ili_draw_bitmap(ili_dev, 16, 16, font_witdh, font_height, FONT_BLANK, NULL, NULL);
		if (r != ESP_OK)
			break;

		vTaskDelay(xTicksToWait);
		r = ili_draw_bitmap(ili_dev, 16, 16, font_witdh, font_height, FONT_SQUARE, NULL, NULL);
		if (r != ESP_OK)
			break;
	}
}

static void lcdui_task(void *params) {
	ESP_LOGD(TAG, "lcdui task starting");
	lcdui_main(params);
	ESP_LOGE(TAG, "lcdui task ending");
	vTaskDelete(NULL);
}

static esp_err_t lcdui_print_string(const char *string, uint16_t x, uint16_t y) {
	esp_err_t r;
	for (int i = 0;; i++) {
		uint8_t c = (uint8_t) string[i];
		if (c == 0)
			break;
		if (c >= 128)
			continue;
		const uint16_t *font = FONT_ASCII[c];
		if (font == NULL)
			continue;

		r = ili_draw_bitmap(ili_dev,
		                    (uint16_t) (x + i * font_witdh), y,
		                    font_witdh, font_height,
		                    font, NULL, NULL);
		if (r != ESP_OK)
			return r;
	}
	return ESP_OK;
}
#endif

esp_err_t lcdui_append_CAN_message(const can_message_t* msg) {
#ifdef CONFIG_LCDUI_ENABLED
#endif
	return ESP_OK;
}

void lcdui_start() {
#ifdef CONFIG_LCDUI_ENABLED
	xTaskCreate(&lcdui_task, "lcdui_task", 3000, NULL, 9, NULL);
#endif
}

