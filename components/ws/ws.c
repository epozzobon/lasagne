#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "mbedtls/base64.h"
#include "mbedtls/sha1.h"

#include "sys/socket.h"
#include "arpa/inet.h"
#include <fcntl.h>

#include "esp_log.h"

#include "string.h"
#include "ws.h"

#define WS_MAX_KEY_LENGTH 32
#define WS_BUF_LENGTH 128

#define WS_STATE_HEADERS 0
#define WS_STATE_SEND_KEY 1
#define WS_STATE_RECV_FRAME 2
#define WS_STATE_RECV_PAYLOAD 3
#define WS_STATE_CLOSED 255

const static char* TAG = "WS";
const static int client_queue_size = 10;
static int sockfd;
static struct sockaddr_in server_addr;

struct ws_client {
	int sockfd;
	uint8_t writable;
	uint8_t state;
	uint8_t buf[WS_BUF_LENGTH];
	size_t buf_len;
	char key[WS_MAX_KEY_LENGTH];
	int pos;
};
typedef struct ws_client *ws_client_t;
const static int max_clients = 10;
static ws_client_t clients[10];
struct ws_client clients_mem[10];

static char* ws_hash_handshake(const char* handshake, char *ret) {
  const char hash[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
  const uint8_t hash_len = sizeof(hash);
  char key[64];
  unsigned char sha1sum[20];
  unsigned int ret_len;
  int len;

  len = strlen(handshake);

  if(!len) return NULL;

  memcpy(key,handshake,len);
  strlcpy(&key[len],hash,sizeof(key));
  mbedtls_sha1((unsigned char*)key,len+hash_len-1,sha1sum);
  mbedtls_base64_encode(NULL, 0, &ret_len, sha1sum, 20);
  if(!mbedtls_base64_encode((unsigned char*)ret,32,&ret_len,sha1sum,20)) {
    ret[ret_len] = '\0';
    return ret;
  }
  return NULL;
}

static void client_parse_line(ws_client_t client, const char *line) {
	const char *key;

	if (line[0] == 0) {
		client->state = 1;
	} else if (line == strstr(line, "Sec-WebSocket-Key: ")) {
		key = line + sizeof("Sec-WebSocket-Key: ") - 1;
		ESP_LOGI(TAG, "Found WebSocket Key: %s", key);
		ws_hash_handshake(key, client->key);
		ESP_LOGI(TAG, "Hashed WebSocket Key: %s", client->key);
	}
}

static int client_extract_frame(ws_client_t client) {
	uint8_t *buf = client->buf;
	int pos = client->pos;
	if (pos < 2) return 0;
	int fin = buf[0] & 0x80;
	int opcode = buf[0] & 0x0f;
	int masked = buf[1] & 0x80;
	uint64_t payload_length = buf[1] & 0x7f;
	int hdr_len = 2;
	uint8_t mask[4] = {0,0,0,0};

	if (payload_length >= 126) {
		hdr_len += 2;
		if (pos < hdr_len) return 0;
		payload_length = buf[2] * 256 + buf[3];
	}

	if (payload_length == 127) {
		hdr_len += 4;
		if (pos < hdr_len) return 0;
		payload_length <<= 32;
		payload_length = (payload_length << 32) +
				(buf[4] << 24) + (buf[5] << 16) + (buf[6] << 8) + buf[7];
	}

	if (masked) {
		hdr_len += 4;
		if (pos < hdr_len) return 0;
		memcpy(mask, buf + hdr_len - 4, 4);
	}

	size_t necessary_buf = hdr_len + payload_length;
	if (client->buf_len < necessary_buf) {
		ESP_LOGW(TAG, "Client %d sent too much data", client->sockfd);
		client->state = WS_STATE_CLOSED;
	}
	if (pos < necessary_buf) return 0;

	uint8_t *payload = client->buf + hdr_len;

	for (int i = 0; i < payload_length; i++) {
		payload[i] ^= mask[i % 4];
	}

	switch (opcode) {
	case 1: {

		ESP_LOGI(TAG, "Client %d sent: %.*s", client->sockfd, (int) payload_length, payload);


	} break;
	default: {
		ESP_LOGW(TAG, "Client %d sent unexpected opcode", client->sockfd);
		client->state = WS_STATE_CLOSED;
	} break;
	}

	uint8_t *next_frame = payload + payload_length;
	size_t rem = pos - (buf - next_frame);
	memcpy(client->buf, next_frame, rem);
	client->pos -= rem;

	return 0;
}

static int client_extract_line(ws_client_t client) {
	const char *line = (const char *) client->buf;

	for (int j = 1; j < client->pos; j++) {
		if (client->buf[j-1] == '\r' && client->buf[j] == '\n') {
			client->buf[j-1] = 0;
			ESP_LOGI(TAG, "recv: %s", client->buf);

			client_parse_line(client, line);

			int rem = client->pos - j - 1;
			uint8_t *next = client->buf + j + 1;
			memcpy(client->buf, next, rem);
			client->pos = rem;
			return 1;
		}
	}

	if (client->pos == client->buf_len) {
		ESP_LOGW(TAG, "recv out of buffer: %.*s", client->buf_len, client->buf);
		if (client->buf[client->buf_len-1] == '\r') {
			client->buf[0] = '\r';
			client->pos = 1;
		} else {
			client->pos = 0;
		}
	}

	return 0;
}

static void client_send_handshake(ws_client_t client) {
	const char WS_RSP_1[] = "HTTP/1.1 101 Switching Protocols\r\n"
			"Upgrade: websocket\r\n"
			"Connection: Upgrade\r\n"
			"Sec-WebSocket-Accept: ";
	const char WS_RSP_2[] = "\r\n\r\n";
	ssize_t r;

	r = send(client->sockfd, WS_RSP_1, sizeof(WS_RSP_1) - 1, 0);
	if (r < 0) goto ERR;
	r = send(client->sockfd, client->key, strlen(client->key), 0);
	if (r < 0) goto ERR;
	r = send(client->sockfd, WS_RSP_2, sizeof(WS_RSP_2) - 1, 0);
	if (r < 0) goto ERR;
	client->state = 2;
	return;

ERR:
	ESP_LOGE(TAG, "Could not send a response to socket %d", client->sockfd);
	client->state = WS_STATE_CLOSED;
	return;
}

void client_handle(ws_client_t client, fd_set *fds) {
	int ret;

	ret = FD_ISSET(client->sockfd, &fds[0]);
	if (ret) {
		ESP_LOGI(TAG, "socket %d becomes readable", client->sockfd);
		ret = recv(client->sockfd, client->buf + client->pos, client->buf_len - client->pos, 0);
		if (ret < 0) {
			ESP_LOGE(TAG, "recv: %s", strerror(errno));
		} else if (ret == 0) {
			ESP_LOGI(TAG, "socket %d closed", client->sockfd);
			client->state = WS_STATE_CLOSED;
			return;
		} else {
			client->pos += ret;
			if (client->state == WS_STATE_HEADERS) {
				while (client_extract_line(client));
			} else if (client->state == WS_STATE_RECV_FRAME) {
				while (client_extract_frame(client));
			}
		}
	}

	ret = FD_ISSET(client->sockfd, &fds[1]);
	if (ret) {
		ESP_LOGI(TAG, "socket %d becomes writeable", client->sockfd);
		client->writable = 1;
	}

	ret = FD_ISSET(client->sockfd, &fds[2]);
	if (ret) {
		ESP_LOGE(TAG, "client socket exception");
		client->state = WS_STATE_CLOSED;
		return;
	}

	if (client->state == 1 && client->writable) {
		client_send_handshake(client);
	}
}


// handles clients when they first connect. passes to a queue
static esp_err_t server_main(void* pvParameters) {
	int ret;
	fd_set fds[3];
	fd_set *readfds;
	fd_set *writefds;
	fd_set *exceptfds;
	struct timeval timeout;
	int clients_num;
	int max_fd;
	socklen_t client_addr_len;
	struct sockaddr_in client_addr;
	ws_client_t client;

	clients_num = 0;
	readfds = &fds[0];
	writefds = &fds[1];
	exceptfds = &fds[2];

	ESP_LOGI(TAG, "websocket server starting");
	sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sockfd < 0) {
		ESP_LOGE(TAG, "server socket creation failed: %s", strerror(errno));
		return ESP_FAIL;
	}

	ret = fcntl(sockfd, F_SETFL, O_NONBLOCK);
	if (ret < 0) {
		ESP_LOGE(TAG, "fcntl: %s", strerror(errno));
		close(sockfd);
		return ESP_FAIL;
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family      = AF_INET;
	server_addr.sin_addr.s_addr = htonl(0);
	server_addr.sin_port        = htons(81);
	ret = bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if (ret < 0) {
		ESP_LOGE(TAG, "listen: %s", strerror(errno));
		close(sockfd);
		return ESP_FAIL;
	}

	ret = listen(sockfd, client_queue_size);
	if (ret < 0) {
		ESP_LOGE(TAG, "listen: %s", strerror(errno));
		close(sockfd);
		return ESP_FAIL;
	}

	for (int i = 0; i < max_clients; i++) {
		clients[i] = &clients_mem[i];
	}

	while (1) {
		max_fd = sockfd;
		FD_ZERO(readfds);
		FD_ZERO(writefds);
		FD_ZERO(exceptfds);
		if (clients_num < max_clients) {
			FD_SET(sockfd, readfds);
		}
		FD_SET(sockfd, exceptfds);
		for (int i = 0; i < clients_num; i++) {
			client = clients[i];
			FD_SET(client->sockfd, readfds);
			FD_SET(client->sockfd, exceptfds);
			if (!client->writable) {
				FD_SET(client->sockfd, writefds);
			}
			max_fd = client->sockfd > max_fd ? client->sockfd : max_fd;
		}

		timeout.tv_sec = 10;
		timeout.tv_usec = 0;
		ret = select(max_fd + 1, readfds, writefds, exceptfds, &timeout);
		ESP_LOGI(TAG, "select returned %d", ret);

		if (FD_ISSET(sockfd, readfds)) {
			ESP_LOGI(TAG, "server socket becomes readable");
			if (clients_num < max_clients) {
				client_addr_len = sizeof(client_addr);
				ret = accept(sockfd, (struct sockaddr *) &client_addr, &client_addr_len);
				if (ret < 0) {
					ESP_LOGE(TAG, "accept: %s", strerror(errno));
				} else {
					client = clients[clients_num++];
					client->sockfd = ret;
					client->writable = 0;
					client->state = WS_STATE_HEADERS;
					client->buf_len = 128;
					client->pos = 0;
					memset(client->key, 0, WS_MAX_KEY_LENGTH);

					ret = fcntl(client->sockfd, F_SETFL, O_NONBLOCK);
					if (ret < 0) {
						ESP_LOGE(TAG, "fcntl: %s", strerror(errno));
						close(sockfd);
						return ESP_FAIL;
					}
				}
			}
		}

		if (FD_ISSET(sockfd, exceptfds)) {
			ESP_LOGE(TAG, "exception happened on server socket");
			close(sockfd);
			return ESP_FAIL;
		}

		for (int i = 0; i < clients_num; i++) {
			client = clients[i];

			client_handle(client, fds);

			if (client->state == WS_STATE_CLOSED) {
				close(clients[i]->sockfd);
				clients_num--;
				clients[i] = clients[clients_num];
				clients[clients_num] = client;
				i--;
			}
		}
	}
}

static void server_task(void* pvParameters) {
	server_main(pvParameters);
	ESP_LOGE(TAG, "task ending");
	vTaskDelete(NULL);
}

static void count_task(void* pvParameters) {
	const static char* TAG = "count_task";
	char out[20];
	int clients;
	uint8_t n = 0;
	const int DELAY = 1000 / portTICK_PERIOD_MS; // 1 second

	ESP_LOGI(TAG, "websocket counter task starting");
	for (;;) {
		clients = 0;
		//clients = ws_server_send_text_all(out, len);
		if (clients > 0) {
			ESP_LOGI(TAG,"sent: \"%s\" to %i clients",out,clients);
		}
		n++;
		vTaskDelay(DELAY);
	}
}

void ws_start() {
	sockfd = -1;
	xTaskCreate(&server_task, "ws_task", 3000, NULL, 9, NULL);
	xTaskCreate(&count_task, "count_task", 6000, NULL, 2, NULL);
}

void ws_stop() {
	close(sockfd);
}
