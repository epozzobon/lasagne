#include <esp_log.h>
#include <malloc.h>
#include <sys/param.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <esp_spiffs.h>
#include "settings.h"
#include "map.h"

struct setting_hdr {
	size_t size;
	settings_listener_func_t listener;
	void *closure;
	void *value;
};

typedef struct setting_hdr *setting_ptr_t;
typedef map_t (setting_ptr_t) map_settings_t;
static map_settings_t map;

static char TAG[] = "Settings";

int settings_init() {
	map_init(&map);

	ESP_LOGI(TAG, "Initializing SPIFFS");

	esp_vfs_spiffs_conf_t conf = {
		.base_path = "/spiffs",
		.partition_label = NULL,
		.max_files = 5,
		.format_if_mount_failed = true
	};

	esp_err_t ret = esp_vfs_spiffs_register(&conf);
	if (ret != ESP_OK) {
		if (ret == ESP_FAIL) {
			ESP_LOGE(TAG, "Failed to mount or format filesystem");
		} else if (ret == ESP_ERR_NOT_FOUND) {
			ESP_LOGE(TAG, "Failed to find SPIFFS partition");
		} else {
			ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
		}
		return -1;
	}

	size_t total = 0, used = 0;
	ret = esp_spiffs_info(NULL, &total, &used);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
	} else {
		ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
	}



	return 0;
}

static int settings_register_allocated(const char *key, setting_ptr_t s) {
	setting_ptr_t *sp;
	int r;
	FILE* f;
	struct stat st;
	char filename[32];

	// Check if this setting is already allocated
	ESP_LOGI(TAG, "Registering %s with size %d", key, s->size);
	sp = map_get(&map, key);
	if (sp != NULL) {
		ESP_LOGE(TAG, "Setting %s already exists", key);
		return -1;
	}

	// Read the value from the file, if it exists
	snprintf(filename, sizeof(filename), "/spiffs/etc/%s", key);
	if (stat(filename, &st) == 0) {
		ESP_LOGI(TAG, "Reading file");
		f = fopen(filename, "rb");
		if (f == NULL) {
			ESP_LOGE(TAG, "Failed to open file for reading");
			return -1;
		}
		fread(s->value, 1, s->size, f);
		fclose(f);
	}

	// Add the setting to the map
	r = map_set(&map, key, s);
	if (r < 0) {
		ESP_LOGE(TAG, "Setting %s could not be set", key);
		return -1;
	}
	return r;
}

int settings_register(const char *key, const void *default_value, size_t size) {
	setting_ptr_t s;
	int r;

	// Allocate some memory for this setting
	s = (setting_ptr_t) malloc(sizeof(*s) + size);
	if (s == NULL) {
		ESP_LOGE(TAG, "Setting %s could not be allocated", key);
		return -1;
	}

	// Set the default values
	s->size = size;
	s->closure = NULL;
	s->listener = NULL;
	s->value = (uint8_t *)s + sizeof(*s);
	memcpy(s->value, default_value, size);

	r = settings_register_allocated(key, s);
	if (r < 0) {
		free(s);
	}
	return r;
}

int settings_subscribe(const char *key, settings_listener_func_t listener, void *closure) {
	setting_ptr_t s, *sp;
	uint8_t *stored_value;

	ESP_LOGI(TAG, "Writing %s", key);
	sp = map_get(&map, key);
	if (sp == NULL) {
		ESP_LOGE(TAG, "Setting %s does not exist", key);
		return -1;
	}
	s = *sp;

	if (s->listener != NULL && listener != NULL) {
		ESP_LOGW(TAG, "Setting %s listener was swapped!", key);
	}

	s->closure = closure;
	s->listener = listener;

	if (s->listener != NULL) {
		stored_value = (uint8_t *)s + sizeof(*s);
		s->listener(key, stored_value, s->size, s->closure);
	}

	return 0;
}

int settings_write(const char *key, const void *value, size_t size) {
	setting_ptr_t s, *sp;
	uint8_t *stored_value;

	ESP_LOGI(TAG, "Writing %s", key);
	sp = map_get(&map, key);
	if (sp == NULL) {
		ESP_LOGE(TAG, "Setting %s does not exist", key);
		return -1;
	}
	s = *sp;

	if (s->size < size) {
		ESP_LOGE(TAG, "Setting %s overflow", key);
		return -1;
	}

	stored_value = (uint8_t *)s + sizeof(*s);

	ESP_LOGD(TAG, "memcpying %d bytes", size);
	memset(stored_value, 0, s->size);
	memcpy(stored_value, value, size);

	char filename[32];
	snprintf(filename, sizeof(filename), "/spiffs/etc/%s", key);
	FILE* f = fopen(filename, "wb");
	if (f == NULL) {
		ESP_LOGE(TAG, "Failed to open file for writing");
		return -1;
	}

	size_t written = fwrite(stored_value, s->size, 1, f);
	fclose(f);
	if (written != 1) {
		ESP_LOGE(TAG, "Failed to write file");
		return -1;
	}

	if (s->listener != NULL) {
		ESP_LOGD(TAG, "calling listener");
		s->listener(key, value, size, s->closure);
	}

	return size;
}

int settings_read(const char *key, void *value, size_t size) {
	setting_ptr_t s;
	uint8_t *stored_value;
	setting_ptr_t *sp;

	ESP_LOGI(TAG, "Querying %s", key);
	sp = map_get(&map, key);
	if (sp == NULL) {
		ESP_LOGE(TAG, "Setting %s does not exist", key);
		return -1;
	}

	s = *sp;
	stored_value = (uint8_t *)s + sizeof(*s);

	if (s->size > size) {
		ESP_LOGE(TAG, "Setting %s is too long", key);
		return -1;
	}

	memcpy(value, stored_value, s->size);
	return s->size;
}
