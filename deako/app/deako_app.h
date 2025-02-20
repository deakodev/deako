#ifndef DEAKO_APP_H
#define DEAKO_APP_H

#include "deako_internal.h"

#include <GLFW/glfw3.h>

#include <stdint.h>

typedef struct dk_config dk_config_t;

typedef void (*dk_on_update_cb)();

typedef struct dk_window {
    dk_module_t module;
    dk_on_request_cb on_request;
} dk_window_t;

typedef struct dk_layer {
    const char* name;
    dk_on_update_cb on_update;
    dk_on_request_cb on_request;
} dk_layer_t;

typedef struct dk_app {
    dk_timer_t timer;
    GLFWwindow* glfw_window;
    dk_layer_t* layers;
    dk_module_t* modules;
    uint32_t layer_count;
    uint32_t module_count;
    uint32_t active_requests;
    bool is_running;
} dk_app_t;

extern int _dk_app_init(const dk_config_t* config);
extern int _dk_app_run(void);
extern int _dk_app_shutdown(void);

extern int _dk_app_status_update(void);
extern void _dk_app_layer_update(void);
extern void _dk_app_time_update(uint64_t* time);

extern int _dk_app_window_init(dk_app_t* app, int width, int height, const char* name);
extern void _dk_app_window_poll(void);

#endif // DEAKO_APP_H