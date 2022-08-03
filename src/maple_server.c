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
        

    server->wl_event_loop = wl_display_get_event_loop(server->wl_display);
    if (server->wl_event_loop == NULL)
    {
        wlr_log(WLR_ERROR, "Failed to connect to event loop");
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
    if (!wlr_backend_start(server->backend)) {
        wlr_log(WLR_ERROR, "Failed to start backend");
        wlr_backend_destroy(server->backend);
        wl_display_destroy(server->wl_display);
        return false;
    }
    
    return true;
}

bool server_destroy(struct maple_server *server) 
{
    

}
