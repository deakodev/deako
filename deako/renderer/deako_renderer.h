#ifndef DEAKO_RENDERER_H
#define DEAKO_RENDERER_H

#include "deako_internal.h"

typedef struct dk_renderer {
    DK_MODULE_FIELDS
    int x;
} dk_renderer_t;

extern int _dk_renderer_init(dk_renderer_t* module);
extern int _dk_renderer_shutdown(void);

extern int _dk_vulkan_init(void);


#endif // DEAKO_RENDERER_H