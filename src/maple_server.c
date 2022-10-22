#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include "maple_server.h"
#include <wlr/util/log.h>

bool server_init(struct maple_server *server) 
{    
    /*
        don't need the if statements but will
        be easier to find issues
    */
    server->wl_display = wl_display_create();
    if (server->wl_display == NULL) 
    {
        wlr_log(WLR_ERROR, "Failed to create to Wayland display");
        return false;
    }

    // will attempt to run in a Wayland/X window or DRM
    server->backend = wlr_backend_autocreate(server->wl_display);
    if (server->backend == NULL)
    {
        wlr_log(WLR_ERROR, "Failed to create backend");
        return false;
    }
        

    return true;
}

bool server_run(struct maple_server *server)
{

    const char *socket = wl_display_add_socket_auto(server->wl_display);

    if (!socket)
    {
        wlr_backend_destroy(server->backend);
        return false;
    }

    if (!wlr_backend_start(server->backend))
    {
        wlr_log(WLR_ERROR, "Failed to start backend");
        wlr_backend_destroy(server->backend);
        wl_display_destroy(server->wl_display);
        return false;
    }

    setenv("WAYLAND_DISPLAY", socket, true);

    return true;
}

bool server_destroy(struct maple_server *server) 
{
    
    wl_display_destroy(server->wl_display);
    return true;
}
