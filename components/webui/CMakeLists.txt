set(COMPONENT_SRCS "webui.c" "page_config.c" "page_ctrl.c" "page_echo.c"
	"page_hello.c" "page_root.c" "page_ws.c" "webui.c" "pages.h")
set(COMPONENT_ADD_INCLUDEDIRS "include")

set(COMPONENT_REQUIRES esp_http_server nvs_flash settings)

register_component()
