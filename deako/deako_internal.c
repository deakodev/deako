#include "deako_internal.h"
#include "deako_pch.h"

#include "app/deako_app.h"
#include "renderer/deako_renderer.h"

#include <stdint.h>

#define DK_ERROR_CASE_MESSAGE(errno, message) \
    case errno: return message;
#define DK_ERROR_CASE_NAME(errno, message) \
    case errno: return #errno;

#define DK_ERROR_CASE_MAP(CASE)             \
    CASE(DK_ERRNO_UNKNOWN, "unknown error") \
    CASE(DK_ERRNO_CANCELED, "null pointer found")

void dk_error_print(dk_errno error, const char* location)
{
    printf("\033[1;31m%s: %s %s\033[0m\n", dk_error_name_string(error), dk_error_message_string(error), location);
}

const char* dk_error_name_string(dk_errno error)
{
    switch (error)
    {
        DK_ERROR_CASE_MAP(DK_ERROR_CASE_NAME)
    }
    return "MAGIC_ERROR_UNKNOWN";
}

const char* dk_error_message_string(dk_errno error)
{
    switch (error)
    {
        DK_ERROR_CASE_MAP(DK_ERROR_CASE_MESSAGE)
    }
    return "unable to identify error type";
}

int _dk_module_init(dk_app_t* app, dk_module_t* module)
{
    switch (module->type)
    {
    case DK_MODULE_TYPE_RENDERER: _dk_renderer_init((dk_renderer_t*)module); break;
    case DK_MODULE_TYPE_UNKNOWN: return DK_ERRNO_UNKNOWN;
    }

    app->modules = module;

    return DK_STATUS_OK;
}

void _dk_module_unref(dk_module_t* module)
{
}

void _dk_modules_on_attach(dk_module_t* module)
{
}