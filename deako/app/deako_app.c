#include "deako_app.h"
#include "deako_pch.h"

#include "deako.h"

#include <malloc.h>
#include <stdint.h>

static dk_app_t* g_app = NULL;

int _dk_app_init(const dk_config_t* config)
{
    DK_CHECK(config, DK_ERRNO_UNKNOWN);

    g_app = malloc(sizeof(*g_app)); // temp
    DK_CHECK(g_app, DK_ERRNO_UNKNOWN);

    g_app->layers      = config->app_layers;
    g_app->layer_count = config->app_layer_count;

    _dk_app_window_init(g_app, config->window_width, config->window_height, config->app_name);

    g_app->timer.timeout  = 0;
    g_app->timer.timestep = 16; // ms
    g_app->timer.callback = _dk_app_layer_update;

    dk_module_t renderer = {
        .name  = "VULKAN_RENDERER",
        .type  = DK_MODULE_TYPE_RENDERER,
        .flags = DK_RENDERER_FLAG_VULKAN,
    };

    int status = _dk_module_init(g_app, &renderer);
    DK_STATUS(status);

    return DK_STATUS_OK;
}

int _dk_app_run(void)
{
    uint64_t time;

    int status = _dk_app_status_update();
    while (status == DK_STATUS_RUN)
    {
        _dk_app_window_poll();

        _dk_app_time_update(&time);
        if (time >= g_app->timer.timeout)
        {
            g_app->timer.timeout = time + g_app->timer.timestep;
            g_app->timer.callback();
            static int frame = 0;
            DK_INFO("app layers updated at: %llu ms (frame %d)\n", g_app->timer.timeout, frame++);
        }

        status = _dk_app_status_update();
    }

    return status;
}

int _dk_app_shutdown(void)
{
    return DK_STATUS_OK;
}

int _dk_app_status_update(void)
{
    return g_app->is_running ? DK_STATUS_RUN : DK_STATUS_OK;
}

void _dk_app_layer_update(void)
{
    for (uint32_t i = 0; i < g_app->layer_count; i++)
    {
        if (g_app->layers[i].on_update)
        {
            g_app->layers[i].on_update();
        }
    }
}

void _dk_app_time_update(uint64_t* time)
{
    *time = (uint64_t)(glfwGetTime() * 1000); // ms
}
