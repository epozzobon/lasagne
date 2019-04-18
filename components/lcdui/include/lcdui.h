#pragma once

#include <driver/can.h>

void lcdui_start();
esp_err_t lcdui_append_CAN_message(const can_message_t* msg);
esp_err_t lcdui_print_string(const char *string, uint16_t x, uint16_t y);

