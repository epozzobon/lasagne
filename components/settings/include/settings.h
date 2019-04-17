#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <stdint.h>
#include <sys/types.h>

typedef void (*settings_listener_func_t)(const char *key, const uint8_t *value, size_t size, void *closure);

int settings_init();
int settings_register(const char *key, const void *value, size_t size);
int settings_subscribe(const char *key, settings_listener_func_t listener, void *closure);
int settings_write(const char *key, const void *value, size_t size);
int settings_read(const char *key, void *value, size_t size);

#endif
