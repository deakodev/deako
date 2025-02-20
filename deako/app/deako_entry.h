#ifndef DEAKO_ENTRY_H
#define DEAKO_ENTRY_H

#include "deako.h"
#include "app/deako_app.h"

int main()
{
    int status;
    const dk_config_t config = dk_configure(); // user-defined
    status                   = _dk_app_init(&config);
    DK_STATUS(status);
    status = _dk_app_run();
    DK_STATUS(status);
    status = _dk_app_shutdown();
    return status;
}

#endif // DEAKO_ENTRY_H
