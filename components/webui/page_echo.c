#include <stdio.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "pages.h"
#include "settings.h"

static char TAG[] = "WEBUI_CTRL";

/* An HTTP POST handler */
esp_err_t echo_post_handler(httpd_req_t *req) {
	char buf[100];
	int ret, remaining = req->content_len;

	while (remaining > 0) {
		/* Read the data for the request */
		if ((ret = httpd_req_recv(req, buf,
		                          MIN(remaining, sizeof(buf)))) < 0) {
			return ESP_FAIL;
		}

		/* Send back the same data */
		httpd_resp_send_chunk(req, buf, ret);
		remaining -= ret;

		/* Log data received */
		ESP_LOGI(TAG, "=========== RECEIVED DATA ==========");
		ESP_LOGI(TAG, "%.*s", ret, buf);
		ESP_LOGI(TAG, "====================================");
	}

	// End response
	httpd_resp_send_chunk(req, NULL, 0);
	return ESP_OK;
}

httpd_uri_t echo = {
	.uri       = "/echo",
	.method    = HTTP_POST,
	.handler   = echo_post_handler,
	.user_ctx  = NULL
};
