/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
 */
#include <stdio.h>
#include <esp_log.h>
#include <esp_system.h>
#include <sys/param.h>

#include "webui.h"
#include "pages.h"

static char TAG[] = "webui";
static httpd_handle_t server;

esp_err_t webserver_login(httpd_req_t *req) {
	int login_successful = 0;
	int buf_len = httpd_req_get_hdr_value_len(req, "Authorization") + 1;
	char buf[200];

	if (buf_len > 1 && buf_len < 200) {
		/* Copy null terminated value string into buffer */
		if (httpd_req_get_hdr_value_str(req, "Authorization", buf, buf_len) == ESP_OK) {
			ESP_LOGI(TAG, "Found header => Authorization: %s", buf);
			if (0 == strcmp(buf, "Basic YWRtaW46Y2FubmVsbG9uaQ==")) {
				login_successful = 1;
			}
		}
	}

	if (!login_successful) {
		httpd_resp_set_type(req, "text/html");
		httpd_resp_set_status(req, "401 Unauthorized");
		httpd_resp_set_hdr(req, "WWW-Authenticate", "Basic realm=\"User Visible Realm\"");
		httpd_resp_send(req, "", 0);
		return ESP_FAIL;
	}

	return ESP_OK;
}

httpd_handle_t start_webserver(void) {
	httpd_handle_t server = NULL;
	httpd_config_t config = HTTPD_DEFAULT_CONFIG();

	// Start the httpd server
	ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
	if (httpd_start(&server, &config) == ESP_OK) {
		// Set URI handlers
		ESP_LOGI(TAG, "Registering URI handlers");
		httpd_register_uri_handler(server, &uri_root);
		httpd_register_uri_handler(server, &hello);
		httpd_register_uri_handler(server, &echo);
		httpd_register_uri_handler(server, &ctrl);
		httpd_register_uri_handler(server, &uri_config);
		httpd_register_uri_handler(server, &uri_ws);
		return server;
	}

	ESP_LOGI(TAG, "Error starting server!");
	return NULL;
}

void stop_webserver(httpd_handle_t server) {
	httpd_stop(server);
}

void webui_start() {
	server = start_webserver();
}

void webui_stop() {
	stop_webserver(server);
}
