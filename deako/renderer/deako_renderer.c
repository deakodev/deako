#include "deako_pch.h"
#include "deako_renderer.h"

#include <malloc.h>

static dk_renderer_t* g_renderer = NULL;

int _dk_renderer_init(dk_renderer_t* module)
{
    g_renderer = malloc(sizeof(*g_renderer));
    DK_CHECK(g_renderer, DK_ERRNO_UNKNOWN);
    *g_renderer = *module;

    switch (g_renderer->flags)
    {
    case DK_RENDERER_FLAG_VULKAN: _dk_vulkan_init(); break;
    default: return DK_ERRNO_UNKNOWN;
    }

    DK_DEBUG("Initialized: %s\n", g_renderer->name);

    g_renderer->x = 3;

    DK_DEBUG("Renderer x: %d\n", g_renderer->x);

    return DK_STATUS_OK;
}

int _dk_renderer_shutdown(void)
{
    return DK_STATUS_OK;
}

int _dk_vulkan_init(void)
{
    DK_DEBUG("Initializing...");
    return DK_STATUS_OK;
}