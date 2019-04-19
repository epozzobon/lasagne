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

httpd_uri_t uri_root = {
	.uri       = "/",
	.method    = HTTP_GET,
	.handler   = root_get_handler,
	.user_ctx  = ""
};

esp_err_t root_get_handler(httpd_req_t *req) {
	char addr_str[16];
	char wifi_ssid[32];
	char wifi_pwd[64];
	char port_str[8];
	char can_baud_str[16];
	int r;
	uint32_t can_baud;
	struct sockaddr_in addr;
	esp_err_t err;

	ESP_LOGI(TAG, "Incoming request");

	err = webserver_login(req);
	if (err != ESP_OK)
		return err;

	httpd_resp_set_type(req, "text/html");
	httpd_resp_set_status(req, "200 OK");

	strcpy(port_str, "?");
	strcpy(addr_str, "?");
	strcpy(wifi_pwd, "?");
	strcpy(wifi_ssid, "?");
	strcpy(can_baud_str, "?");

	settings_read("wifi_password", wifi_pwd, sizeof(wifi_pwd));
	settings_read("wifi_ssid", wifi_ssid, sizeof(wifi_ssid));

	r = settings_read("cannelloni_remote", &addr, sizeof(addr));
	if (r >= 0) {
		inet_ntop(AF_INET, &addr.sin_addr, addr_str, sizeof(addr_str));
		snprintf(port_str, sizeof(port_str), "%u", htons(addr.sin_port));
	}

	r = settings_read("can_baud", &can_baud, sizeof(can_baud));
	if (r >= 0) {
		snprintf(can_baud_str, sizeof(can_baud_str), "%u", can_baud);
	}


	HTTPD_RESP_SEND_STR(req, QUOTE(
							<html><head><title>CAN Lasagne</title></head><body>
							<h1>Welcome</h1>
							<h2>Configuration</h2>
							<h3>Wi-Fi settings</h3>
							<form action="config" method="POST">
							                              <p><label for="wifi_ssid">Wi-Fi SSID</label>
							                                             <input type="text" id="wifi_ssid" name="wifi_ssid"
							));

	HTTPD_RESP_SEND_STR(req, " value=\"");
	httpd_resp_send_chunk(req, wifi_ssid, strlen(wifi_ssid));
	HTTPD_RESP_SEND_STR(req, "\" ");

	HTTPD_RESP_SEND_STR(req, QUOTE(
							/></p>
							<p><label for="wifi_pwd">Wi-Fi password</label>
							               <input type="password" id="wifi_pwd" name="wifi_pwd"
							));

	HTTPD_RESP_SEND_STR(req, " value=\"");
	httpd_resp_send_chunk(req, wifi_pwd, strlen(wifi_pwd));
	HTTPD_RESP_SEND_STR(req, "\" ");

	HTTPD_RESP_SEND_STR(req, QUOTE(
							/></p>
							<input type="submit" name="submit" value="Change Wi-Fi settings" />
							                                          </form><h3>Cannelloni settings</h3>
							                                          <form action="config" method="POST">
							                                                                        <p><label for="cannelloni_ip">Cannelloni IP</label>
							                                                                                       <input type="text" id="cannelloni_ip" name="cannelloni_ip"
							));

	HTTPD_RESP_SEND_STR(req, " value=\"");
	httpd_resp_send_chunk(req, addr_str, strlen(addr_str));
	HTTPD_RESP_SEND_STR(req, "\" ");

	HTTPD_RESP_SEND_STR(req, QUOTE(
							/></p>
							<p><label for="cannelloni_port">Cannelloni port</label>
							               <input type="text" id="cannelloni_port" name="cannelloni_port"
							));

	HTTPD_RESP_SEND_STR(req, " value=\"");
	httpd_resp_send_chunk(req, port_str, strlen(port_str));
	HTTPD_RESP_SEND_STR(req, "\" ");

	HTTPD_RESP_SEND_STR(req, QUOTE(
							/></p>
							<input type="submit" name="submit" value="Change Cannelloni settings" />
							                                          </form><h3>CAN settings</h3>
							                                          <form action="config" method="POST">
							                                                                        <p><label for="can_baud">CAN baud rate</label>
							                                                                                       <input type="text" id="can_baud" name="can_baud"
							));

	HTTPD_RESP_SEND_STR(req, " value=\"");
	httpd_resp_send_chunk(req, can_baud_str, strlen(can_baud_str));
	HTTPD_RESP_SEND_STR(req, "\" ");

	HTTPD_RESP_SEND_STR(req, QUOTE(
							/></p>
							<input type="submit" name="submit" value="Change CAN settings" />
							                                          </form>
							                                          </body></html>
							));
	httpd_resp_send_chunk(req, NULL, 0);

	return ESP_OK;
}
