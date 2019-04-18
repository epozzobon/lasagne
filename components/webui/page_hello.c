#include <stdio.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "pages.h"
#include "settings.h"

static char TAG[] = "WEBUI_HELLO";

/* An HTTP GET handler */
esp_err_t hello_get_handler(httpd_req_t *req) {
	char*  buf;
	size_t buf_len;

	/* Get header value string length and allocate memory for length + 1,
	 * extra byte for null termination */
	buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
	if (buf_len > 1) {
		buf = malloc(buf_len);
		/* Copy null terminated value string into buffer */
		if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK) {
			ESP_LOGI(TAG, "Found header => Host: %s", buf);
		}
		free(buf);
	}

	buf_len = httpd_req_get_hdr_value_len(req, "Test-Header-2") + 1;
	if (buf_len > 1) {
		buf = malloc(buf_len);
		if (httpd_req_get_hdr_value_str(req, "Test-Header-2", buf, buf_len) == ESP_OK) {
			ESP_LOGI(TAG, "Found header => Test-Header-2: %s", buf);
		}
		free(buf);
	}

	buf_len = httpd_req_get_hdr_value_len(req, "Test-Header-1") + 1;
	if (buf_len > 1) {
		buf = malloc(buf_len);
		if (httpd_req_get_hdr_value_str(req, "Test-Header-1", buf, buf_len) == ESP_OK) {
			ESP_LOGI(TAG, "Found header => Test-Header-1: %s", buf);
		}
		free(buf);
	}

	/* Read URL query string length and allocate memory for length + 1,
	 * extra byte for null termination */
	buf_len = httpd_req_get_url_query_len(req) + 1;
	if (buf_len > 1) {
		buf = malloc(buf_len);
		if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
			ESP_LOGI(TAG, "Found URL query => %s", buf);
			char param[32];
			/* Get value of expected key from query string */
			if (httpd_query_key_value(buf, "query1", param, sizeof(param)) == ESP_OK) {
				ESP_LOGI(TAG, "Found URL query parameter => query1=%s", param);
			}
			if (httpd_query_key_value(buf, "query3", param, sizeof(param)) == ESP_OK) {
				ESP_LOGI(TAG, "Found URL query parameter => query3=%s", param);
			}
			if (httpd_query_key_value(buf, "query2", param, sizeof(param)) == ESP_OK) {
				ESP_LOGI(TAG, "Found URL query parameter => query2=%s", param);
			}
		}
		free(buf);
	}

	/* Set some custom headers */
	httpd_resp_set_hdr(req, "Custom-Header-1", "Custom-Value-1");
	httpd_resp_set_hdr(req, "Custom-Header-2", "Custom-Value-2");

	/* Send response with custom headers and body set as the
	 * string passed in user context*/
	const char* resp_str = (const char*) req->user_ctx;
	httpd_resp_send(req, resp_str, strlen(resp_str));

	/* After sending the HTTP response the old HTTP request
	 * headers are lost. Check if HTTP request headers can be read now. */
	if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
		ESP_LOGI(TAG, "Request headers lost");
	}
	return ESP_OK;
}

httpd_uri_t hello = {
	.uri       = "/hello",
	.method    = HTTP_GET,
	.handler   = hello_get_handler,
	/* Let's pass response string in user
	 * context to demonstrate it's usage */
	.user_ctx  = "Hello World!"
};
