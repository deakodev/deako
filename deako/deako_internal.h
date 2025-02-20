#ifndef DEAKO_TYPES_H
#define DEAKO_TYPES_H

#include <log.h>
#include <magic_memory.h> // TODO: temp?

#include <stdint.h>

#define DK_TRACE(...) log_log(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define DK_DEBUG(...) log_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define DK_INFO(...) log_log(LOG_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define DK_WARN(...) log_log(LOG_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define DK_ERROR(...) log_log(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define DK_FATAL(...) log_log(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

#define DK_STATUS_RUN 1
#define DK_STATUS_OK 0

#define DK_STRING(s) #s
#define DK_STRINGIFY(s) DK_STRING(s)

#define DK_ERROR_LINE "in " __FILE__ " line " DK_STRINGIFY(__LINE__)
#define DK_ERROR_PRINT(error)                   \
    {                                           \
        dk_error_print((error), DK_ERROR_LINE); \
    }
#define DK_ERROR_HANDLE(error)   \
    {                            \
        DK_ERROR_PRINT((error)); \
        return (error);          \
    }

#define DK_CHECK(condition, error)    \
    do                                \
    {                                 \
        if (!(condition))             \
        {                             \
            DK_ERROR_HANDLE((error)); \
        }                             \
    } while (0)
#define DK_STATUS(status)            \
    do                               \
    {                                \
        if ((status) < DK_STATUS_OK) \
        {                            \
            return (status);         \
        }                            \
    } while (0)

typedef enum dk_errno {
    DK_ERRNO_UNKNOWN  = -100,
    DK_ERRNO_CANCELED = -101,
} dk_errno;

extern void dk_error_print(dk_errno error, const char* location);
extern const char* dk_error_name_string(dk_errno error);
extern const char* dk_error_message_string(dk_errno error);

typedef Magic_Arena dk_arena_t; // TODO: rename Magic_Arena

typedef void (*dk_on_attach_cb)(void);
typedef void (*dk_on_detach_cb)(void);
typedef void (*dk_on_request_cb)(void);
typedef void (*dk_timer_cb)(void);

#define DK_MODULE_FIELDS         \
    const char* name;            \
    dk_arena_t arena;            \
    dk_module_type type;         \
    dk_on_attach_cb on_attach;   \
    dk_on_detach_cb on_detach;   \
    dk_on_request_cb on_request; \
    uint32_t flags;

typedef enum dk_module_type {
    DK_MODULE_TYPE_UNKNOWN = 0,
    DK_MODULE_TYPE_APP,
    DK_MODULE_TYPE_RENDERER,
} dk_module_type;

typedef enum dk_module_flag {
    DK_MODULE_TYPE_NONE     = 0,
    DK_RENDERER_FLAG_VULKAN = 1 << 0,
} dk_module_flag;

typedef enum dk_handle_type {
    DK_HANDLE_TYPE_UNKNOWN = 0,
} dk_handle_type;

typedef enum dk_handle_flag {
    DK_HANDLE_FLAG_INTERNAL = 1 << 0,
    DK_HANDLE_FLAG_ACTIVE   = 1 << 1,
    DK_HANDLE_FLAG_REF      = 1 << 2,
    DK_HANDLE_FLAG_CLOSING  = 1 << 3
} dk_handle_flag;

typedef enum dk_request_type {
    DK_REQUEST_TYPE_UNKNOWN = 0,
} dk_request_type;

typedef struct dk_module {
    DK_MODULE_FIELDS
} dk_module_t;

typedef struct dk_handle {
    dk_handle_type type;
} dk_handle_t;

typedef struct dk_timer {
    uint64_t timeout;
    uint64_t timestep;
    dk_timer_cb callback;
} dk_timer_t;

typedef struct dk_request {
    dk_request_type type;
} dk_request_t;

typedef struct dk_app dk_app_t;

extern int _dk_module_init(dk_app_t* app, dk_module_t* module);
extern void _dk_module_unref(dk_module_t* module);

extern void _dk_modules_on_attach(dk_module_t* module);

#endif // DEAKO_TYPES_H