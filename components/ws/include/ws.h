#pragma once

void ws_start();
void ws_stop();
int ws_send_all(const uint8_t *payload, uint64_t payload_length, int opcode);
