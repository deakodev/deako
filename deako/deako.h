#ifndef DEAKO_H
#define DEAKO_H

#include "app/deako_app.h"

#include <log.h>
#define DK_APP_TRACE(...) log_log(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define DK_APP_DEBUG(...) log_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define DK_APP_INFO(...)  log_log(LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define DK_APP_WARN(...)  log_log(LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define DK_APP_ERROR(...) log_log(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define DK_APP_FATAL(...) log_log(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

typedef struct dk_config {
	const char* app_name;
	dk_layer_t* app_layers;
	uint32_t app_layer_count;
	int window_width;
	int window_height;
} dk_config_t;

/* user-defined */
extern dk_config_t dk_configure(void);

#ifdef DEAKO_IMPLEMENT_MAIN
#include "app/deako_entry.h"
#endif

#endif // DEAKO_H