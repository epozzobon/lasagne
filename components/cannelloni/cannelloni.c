#include <esp_log.h>
#include <esp_system.h>
#include <sys/param.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <sdkconfig.h>
#include <driver/gpio.h>
#include <driver/can.h>
#include "settings.h"
#include "cannelloni.h"

#define CANNELLONI_DATA_PACKET_BASE_SIZE 5
#define CANNELLONI_FRAME_BASE_SIZE 5
#define CANNELLONI_UDP_RX_PACKET_BUF_LEN 1600
#define CANNELLONI_FRAME_VERSION 2
#define CAN_EFF_FLAG 0x80000000U
#define CAN_RTR_FLAG 0x40000000U
#define CAN_ERR_FLAG 0x20000000U
#define CAN_SFF_MASK 0x000007FFU
#define CAN_EFF_MASK 0x1FFFFFFFU
#define CAN_ERR_MASK 0x1FFFFFFFU

enum op_codes {DATA, ACK, NACK};

struct __attribute__((__packed__)) CannelloniDataPacket {
  uint8_t version;
  uint8_t op_code;
  uint8_t seq_no;
  uint16_t count;
};

static char TAG[] = "CAN";
static TaskHandle_t cannelloni_can_task_handle;
static TaskHandle_t cannelloni_udp_task_handle;
static uint8_t udp_rx_packet_buf[CANNELLONI_UDP_RX_PACKET_BUF_LEN];
static uint8_t udp_tx_packet_buf[CANNELLONI_DATA_PACKET_BASE_SIZE + CANNELLONI_FRAME_BASE_SIZE + 8];
static struct sockaddr_in udp_client;
static uint8_t seq_no;
static int sockfd;

ssize_t cannelloni_build_packet(uint8_t *buffer, can_message_t *frames, int frameCount) {
	struct CannelloniDataPacket *snd_hdr;
	uint8_t *raw_data;
	uint32_t can_id;
	int i;
	size_t packet_size = 0;

	snd_hdr = (struct CannelloniDataPacket *) buffer;
	snd_hdr->count = 0;
	snd_hdr->op_code = DATA;
	snd_hdr->seq_no = seq_no++;
	snd_hdr->version = CANNELLONI_FRAME_VERSION;
	snd_hdr->count = htons(frameCount);

	raw_data = buffer + CANNELLONI_DATA_PACKET_BASE_SIZE;

	for (i = 0; i < frameCount; i++) {
		can_id = frames[i].identifier;
		if (frames[i].flags & CAN_MSG_FLAG_EXTD) {
			can_id |= CAN_EFF_FLAG;
		}
		if (frames[i].flags & CAN_MSG_FLAG_RTR) {
			can_id |= CAN_RTR_FLAG;
		}

		can_id = htonl(can_id);
		memcpy(raw_data, &can_id, sizeof(can_id));
		raw_data += sizeof (can_id);

		*raw_data = frames[i].data_length_code;
		raw_data += sizeof (frames[i].data_length_code);

		if ((frames[i].flags & CAN_MSG_FLAG_RTR) == 0) {
			memcpy(raw_data, frames[i].data, frames[i].data_length_code);
			raw_data += frames[i].data_length_code;
		}
	}

	packet_size = raw_data - buffer;
	return packet_size;
}


#define CAN_TIMING_CONFIG_33300BITS()     {.brp = 120, .tseg_1 = 15, .tseg_2 = 4, .sjw = 3, .triple_sampling = false}

void cannelloni_can_main() {
    //Initialize configuration structures using macro initializers
    can_general_config_t g_config = CAN_GENERAL_CONFIG_DEFAULT(GPIO_NUM_15, GPIO_NUM_4, CAN_MODE_NORMAL);
    can_timing_config_t t_config = CAN_TIMING_CONFIG_33300BITS();
    can_filter_config_t f_config = CAN_FILTER_CONFIG_ACCEPT_ALL();
    esp_err_t err;
	can_message_t message;
	int s;
    int r;

    //Install CAN driver
    err = can_driver_install(&g_config, &t_config, &f_config);
    if (err == ESP_OK) {
    	ESP_LOGI(TAG, "Driver installed");
    } else {
    	ESP_LOGE(TAG, "Failed to install driver");
		return;
    }

    //Start CAN driver
    err = can_start();
    if (err == ESP_OK) {
    	ESP_LOGI(TAG, "Driver started");
    } else {
    	ESP_LOGE(TAG, "Failed to start driver");
		return;
    }

    seq_no = 0;
    while (1) {

		err = can_receive(&message, (10000 / portTICK_PERIOD_MS));
		if (err == ESP_OK) {
			ESP_LOGI(TAG, "CAN message received");

        	r = cannelloni_build_packet(udp_tx_packet_buf, &message, 1);
        	if (r < 0) {
        		ESP_LOGW(TAG, "cannelloni_build_packet failed");
        	}

	        if ((s = sockfd) >= 0) {
	        	r = sendto(s, udp_tx_packet_buf, r, 0, (struct sockaddr *) &udp_client, sizeof(udp_client));
		        if (r < 0) {
		        	ESP_LOGW(TAG, "sendto: %s", strerror(errno));
		        } else {
		        	ESP_LOGI(TAG, "UDP message (%dB) sent to %08x:%d", r, ntohl(udp_client.sin_addr.s_addr), ntohs(udp_client.sin_port));
		        }
	        }

		} else if (err == ESP_ERR_TIMEOUT) {
		} else {
			ESP_LOGE(TAG, "Failed to receive CAN message");
			return;
		}
    }
}

int cannelloni_udp_onrecv(struct sockaddr_in *client_addr, uint8_t *buffer, size_t len) {
	struct CannelloniDataPacket *rcv_hdr;
	char client_addr_str[INET_ADDRSTRLEN];
    const uint8_t* raw_data = buffer + CANNELLONI_DATA_PACKET_BASE_SIZE;
    int received_frames_count;
    can_message_t frame;
    const char *client_addr_str_ptr;
    esp_err_t err;

    client_addr_str_ptr = inet_ntop(AF_INET, &client_addr->sin_addr, client_addr_str, INET_ADDRSTRLEN);
	if (client_addr_str_ptr == NULL) {
    	ESP_LOGW(TAG, "Could not convert client address");
    	return -1;
	}

	if (len < CANNELLONI_DATA_PACKET_BASE_SIZE) {
		ESP_LOGE(TAG, "Did not receive enough data");
		return -1;
	}

	rcv_hdr = (struct CannelloniDataPacket *) buffer;
	if (rcv_hdr->version != CANNELLONI_FRAME_VERSION) {
		ESP_LOGE(TAG, "Received wrong version");
		return -1;
	}

	if (rcv_hdr->op_code != DATA) {
		ESP_LOGE(TAG, "Received wrong OP code");
		return -1;
	}

	received_frames_count = ntohs(rcv_hdr->count);
	if (received_frames_count == 0) {
		return 0;
	}

    for (uint16_t i = 0; i < received_frames_count; i++) {
        if (raw_data - buffer + CANNELLONI_FRAME_BASE_SIZE > len) {
    		ESP_LOGE(TAG, "Received incomplete packet");
    		return -1;
        }

        uint32_t can_id;
        memcpy(&can_id, raw_data, sizeof(can_id));
        can_id = ntohl(can_id);
        raw_data += sizeof (can_id);

        frame.flags = CAN_MSG_FLAG_NONE;
        if (can_id & CAN_ERR_FLAG) {
        	// TODO: what?
        }
		if (can_id & CAN_EFF_FLAG) {
        	frame.flags |= CAN_MSG_FLAG_EXTD;
		}
		if (can_id & CAN_RTR_FLAG) {
        	frame.flags |= CAN_MSG_FLAG_RTR;
		}

		if (frame.flags & CAN_MSG_FLAG_EXTD) {
			frame.identifier = can_id & CAN_EFF_MASK;
		} else {
			frame.identifier = can_id & CAN_SFF_MASK;
		}

        frame.data_length_code = *raw_data;
        raw_data += sizeof (frame.data_length_code);

        if ((frame.flags & CAN_MSG_FLAG_RTR) == 0) {
            if (raw_data - buffer + frame.data_length_code > len)
            {
                ESP_LOGE(TAG, "Received incomplete packet / can header corrupt!");
        		return -1;
            }

            memcpy(frame.data, raw_data, frame.data_length_code);
            raw_data += frame.data_length_code;
        }

		ESP_LOGI(TAG, "CAN message received over UDP");
		err = can_transmit(&frame, 0);
		if (err == ESP_ERR_TIMEOUT) {
			ESP_LOGW(TAG, "Timed out waiting for space on TX queue");
		} else if (err == ESP_FAIL) {
			ESP_LOGW(TAG, "TX queue is disabled and another message is currently transmitting");
		} else if (err != ESP_OK) {
			ESP_LOGE(TAG, "Unsuccessful transmission: %d", err);
			return -1;
		}
    }

	return received_frames_count;
}

void cannelloni_udp_main() {
	int ret;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	socklen_t client_addr_len;

	sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sockfd < 0) {
		ESP_LOGE(TAG, "socket: %s", strerror(errno));
		return;
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family      = AF_INET;
	server_addr.sin_addr.s_addr = htonl(0);
	server_addr.sin_port        = htons(20000);
	ret = bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if (ret < 0) {
		ESP_LOGE(TAG, "bind: %s", strerror(errno));
		close(sockfd);
		return;
	}

	while (1) {
		ret = recvfrom(sockfd, udp_rx_packet_buf, CANNELLONI_UDP_RX_PACKET_BUF_LEN, 0, (struct sockaddr *) &client_addr, &client_addr_len);
		if (ret < 0) {
	    	ESP_LOGE(TAG, "Failed to receive UDP message");
			return;
		}

		ESP_LOGI(TAG, "UDP message received");
		cannelloni_udp_onrecv(&client_addr, udp_rx_packet_buf, ret);
	}
}

void cannelloni_can_task(void *pvParameter)
{
	cannelloni_can_main();
	vTaskDelete(NULL);
}

void cannelloni_udp_task(void *pvParameter)
{
	cannelloni_udp_main();
	vTaskDelete(NULL);
}

void cannelloni_config_listener(const char *key, const uint8_t *value, size_t size, void *closure)
{
	memcpy(&udp_client, value, MIN(sizeof(udp_client), size));
}

void cannelloni_start()
{
    xTaskCreate(&cannelloni_udp_task, "cannelloni_udp_task", 2048, NULL, 5, &cannelloni_udp_task_handle);
}

void cannelloni_stop()
{
    vTaskDelete(cannelloni_udp_task_handle);
	close(sockfd);
	sockfd = -1;
}

void cannelloni_init()
{
	memset(&udp_client, 0, sizeof(udp_client));
	udp_client.sin_family      = AF_INET;
	udp_client.sin_addr.s_addr = htonl(0xac131428);
	udp_client.sin_port        = htons(20000);

	sockfd = -1;
	settings_register("cannelloni_remote", &udp_client, sizeof(udp_client));
	settings_subscribe("cannelloni_remote", cannelloni_config_listener, NULL);

    xTaskCreate(&cannelloni_can_task, "cannelloni_can_task", 2048, NULL, 5, &cannelloni_can_task_handle);
}
