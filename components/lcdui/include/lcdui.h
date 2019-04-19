#pragma once

#include <driver/can.h>

void lcdui_start();
esp_err_t lcdui_append_CAN_message(const can_message_t* msg);

