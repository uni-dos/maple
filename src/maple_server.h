#ifndef MAPLE_SERVER
#define MAPLE_SERVER

#include <wayland-server-core.h>
#include <wlr/backend.h>

// everything needed to be a compositor
// the server is the compositor
struct maple_server {
    
    struct wl_display *wl_display;
    struct wl_event_loop *wl_event_loop;

    struct wlr_backend *backend;
    
};

void server_init(struct maple_server *server);

void server_destroy(struct maple_server *server);


#endif
