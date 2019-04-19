#include <stdio.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "pages.h"
#include "settings.h"

#define STRLEN(s) (sizeof(s)/sizeof(s[0])-1)
#define HTTPD_RESP_SEND_STR(req, s) (httpd_resp_send_chunk(req, s, STRLEN(s)))
#define QUOTE(...) #__VA_ARGS__

static char TAG[] = "WEBUI_ROOT";

httpd_uri_t uri_ws = {
	.uri       = "/ws",
	.method    = HTTP_GET,
	.handler   = ws_get_handler,
	.user_ctx  = ""
};

esp_err_t ws_get_handler(httpd_req_t *req) {
	esp_err_t err;

	ESP_LOGI(TAG, "Incoming request");

	err = webserver_login(req);
	if (err != ESP_OK)
		return err;

	httpd_resp_set_type(req, "text/html");
	httpd_resp_set_status(req, "200 OK");

	HTTPD_RESP_SEND_STR(req, QUOTE(
							<html><head><title>CAN Lasagne</title></head><body>
							<h1>Welcome</h1>
							<script>)
	                    "var domain = /^[A-Z0-9]+:\\/\\/([^:\\/]+)/i.exec(window.location)[1];"
	                    "var exampleSocket = new WebSocket('ws://' + domain + ':81/ws');"
	                    "exampleSocket.onopen = function (event) {"
	                    "exampleSocket.send(\"Here's some text that the server is urgently awaiting!\");"
	                    "var a = 0;"
	                    "setInterval(function () {"
	                    "exampleSocket.send('message ' + (a++));"
	                    "}, 1000);"
	                    "};"
	                    "exampleSocket.onmessage = function (event) {"
	                    "console.log(event.data);"
	                    "}"
	                    QUOTE(
							</script>
							</body></html>
							));
	httpd_resp_send_chunk(req, NULL, 0);

	return ESP_OK;
}
