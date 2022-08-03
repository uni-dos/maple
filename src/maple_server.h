#ifndef MAPLE_SERVER
#define MAPLE_SERVER

#include <wayland-server-core.h>
#include <wlr/backend.h>

// everything needed to be a wayland compositor
// the server is the wm; holds its state
struct maple_server {
    
    struct wl_display *wl_display;
    struct wl_event_loop *wl_event_loop;

    struct wlr_backend *backend;
    
};

// initilize most of the parameters in server
bool server_init(struct maple_server *server);

bool server_run(struct maple_server *server);

bool server_destroy(struct maple_server *server);


#endif
