#ifndef _WEBUI_PAGES_H_
#define _WEBUI_PAGES_H_

#include "webui.h"

esp_err_t webserver_login(httpd_req_t *req);

esp_err_t root_get_handler(httpd_req_t *req);
httpd_uri_t uri_root;

esp_err_t ws_get_handler(httpd_req_t *req);
httpd_uri_t uri_ws;

esp_err_t config_post_handler(httpd_req_t *req);
httpd_uri_t uri_config;

esp_err_t hello_get_handler(httpd_req_t *req);
httpd_uri_t hello;

esp_err_t ctrl_put_handler(httpd_req_t *req);
httpd_uri_t ctrl;

esp_err_t echo_post_handler(httpd_req_t *req);
httpd_uri_t echo;

#endif
