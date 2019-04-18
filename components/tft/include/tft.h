#pragma once

#include "driver/spi_master.h"

typedef void (*ili_draw_cb_t)(void *user);

typedef struct ili_config_t {
	int dc_io_num;
	int spics_io_num;
	int bckl_io_num;
} ili_config_t;

typedef struct ili_device_t {
	spi_device_handle_t spi;
	uint8_t dc_io_num;
	uint8_t bckl_io_num;
} ili_device_t;

typedef struct ili_device_t *ili_device_handle_t;

esp_err_t ili_bus_add_device(spi_host_device_t host, ili_config_t *dev_config, ili_device_handle_t *handle);

esp_err_t ili_draw_bitmap(ili_device_handle_t ili, uint16_t x, uint16_t y,
                          uint16_t w, uint16_t h, const uint16_t *bitmap, ili_draw_cb_t finish_cb, void *user);

void ili_cmd(ili_device_handle_t ili, const uint8_t cmd);
void ili_data(ili_device_handle_t ili, const uint8_t *data, int len);
void ili_spi_pre_transfer_callback(spi_transaction_t *t);
esp_err_t ili_init(ili_device_handle_t ili);
void ili_result_task(void *pvParameter);