#include <stdio.h>
#include <esp_log.h>
#include <esp_system.h>
#include <sys/param.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "pages.h"
#include "settings.h"

#define STRLEN(s) (sizeof(s)/sizeof(s[0])-1)
#define HTTPD_RESP_SEND_STR(req, s) (httpd_resp_send_chunk(req, s, STRLEN(s)))
#define QUOTE(...) #__VA_ARGS__

static char TAG[] = "WEBUI_ROOT";

httpd_uri_t uri_config = {
	.uri       = "/config",
	.method    = HTTP_POST,
	.handler   = config_post_handler,
	.user_ctx  = NULL
};

static esp_err_t post_parameter_received(httpd_req_t *req, const char *key, const char *value) {
	int r;
	ESP_LOGI(TAG, "POST KEY: %s", key);
	ESP_LOGI(TAG, "POST VALUE: %s", value);

	if (0 == strcmp(key, "wifi_ssid")) {
		r = settings_write("wifi_ssid", value, strlen(value) + 1);
		return r < 0 ? ESP_ERR_INVALID_ARG : ESP_OK;
	} else if (0 == strcmp(key, "wifi_pwd")) {
		r = settings_write("wifi_password", value, strlen(value) + 1);
		return r < 0 ? ESP_ERR_INVALID_ARG : ESP_OK;
	} else if (0 == strcmp(key, "cannelloni_ip")) {
		struct sockaddr_in addr;
		struct in_addr inp;

		r = settings_read("cannelloni_remote", &addr, sizeof(addr));
		if (r < 0) return ESP_ERR_NOT_FOUND;

		r = inet_aton(value, &inp);
		if (r == 0) return ESP_ERR_INVALID_ARG;

		addr.sin_addr = inp;
		r = settings_write("cannelloni_remote", &addr, sizeof(addr));
		return r < 0 ? ESP_ERR_INVALID_ARG : ESP_OK;
	} else if (0 == strcmp(key, "cannelloni_port")) {
		struct sockaddr_in addr;
		char *invalid;
		long port;

		r = settings_read("cannelloni_remote", &addr, sizeof(addr));
		if (r < 0) return ESP_ERR_NOT_FOUND;

		port = strtol(value, &invalid, 10);
		if (invalid[0] != 0) return ESP_ERR_INVALID_ARG;

		addr.sin_port = htons((short) port);
		r = settings_write("cannelloni_remote", &addr, sizeof(addr));
		return r < 0 ? ESP_ERR_INVALID_ARG : ESP_OK;
	} else if (0 == strcmp(key, "can_baud")) {
		uint32_t can_baud;
		char *invalid;

		r = settings_read("can_baud", &can_baud, sizeof(can_baud));
		if (r < 0) return ESP_ERR_NOT_FOUND;

		can_baud = strtol(value, &invalid, 10);
		if (invalid[0] != 0) return ESP_ERR_INVALID_ARG;

		r = settings_write("can_baud", &can_baud, sizeof(can_baud));
		return r < 0 ? ESP_ERR_INVALID_ARG : ESP_OK;
	}

	return ESP_ERR_NOT_FOUND;
}

static esp_err_t post_parse_param(httpd_req_t *req, char *buf, size_t len) {
	/* print it on the web page */
	if (len) {
		httpd_resp_send_chunk(req, (char *) buf, len);
		httpd_resp_send_chunk(req, "\n", 1);

		ESP_LOGI(TAG, "============= SENT DATA ============");
		ESP_LOGI(TAG, "%.*s", len, buf);
		ESP_LOGI(TAG, "====================================");
	}

	/* write the value in settings */
	buf[len] = 0;
	char *match = strchr(buf, '=');
	if (match != NULL) {
		match[0] = 0;
		post_parameter_received(req, buf, match+1);
		return ESP_OK;
	} else {
		//post_parameter_received(req, buf, NULL);
		return ESP_ERR_INVALID_ARG;
	}
}

static esp_err_t parse_post_req(httpd_req_t *req) {
	uint8_t buf[101];

	int bw = 0;  // buffer write position
	int br = 0;  // buffer read position
	int remaining = req->content_len;

	while (!(remaining == 0 && br == bw)) {
		/* top off the buffer */
		int toread = MIN(remaining, sizeof(buf) - 1 - bw);
		if (toread > 0) {
			int ret = httpd_req_recv(req, (char *) buf + bw, toread);
			if (ret < 0) {
				return ESP_FAIL;
			} else if (ret == 0) {
				ESP_LOGE(TAG, "Read 0 from request when %dB were remaining", remaining);
				return ESP_ERR_INVALID_STATE;
			}

			bw += ret;
			remaining -= ret;
		}

		/* find next separator, or the end of the buffer */
		for (br = 0; br < bw; br++) {
			if (buf[br] == '&') {
				break;
			}
		}
		if (br > 0) {
			post_parse_param(req, (char *) buf, br);
		}

		/* skip & */
		if (br < bw) {
			br++;
		}

		/* skip any consecutive & */
		while (br < bw && buf[br] == '&') {
			br++;
		}

		/* move the buffer forward */
		memcpy(buf, buf + br, bw - br);
		bw -= br;
		br = 0;
	}

	return ESP_OK;
}

esp_err_t config_post_handler(httpd_req_t *req) {

	parse_post_req(req);

	// End response
	httpd_resp_send_chunk(req, NULL, 0);
	return ESP_OK;
}
