#ifndef MAPLE_OUTPUT
#define MAPLE_OUTPUT

#include "server.h"
#include "wlr/types/wlr_output.h"
#include <wayland-server-core.h>

/* holds the state for the output/display */
struct maple_output {

    /* the actual display */
    struct wlr_output *wlr_output;
    struct maple_server *server;

    struct wl_listener destroy;
    struct wl_listener frame;

    struct wl_list link;

};

void set_up_output(struct maple_server *server);
#endif
