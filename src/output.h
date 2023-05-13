#ifndef MAPLE_OUTPUT_H
#define MAPLE_OUTPUT_H

#include <wayland-server-core.h>
#include <wlr/types/wlr_output.h>
#include "server.h"

/* holds the state for the output/display */
struct maple_output {

    struct wl_list link;

    /* the actual display */
    struct wlr_output *wlr_output;
    struct maple_server *server;

    struct wl_listener destroy;
    struct wl_listener request_state;
    struct wl_listener frame;

};

void set_up_output(struct maple_server *server);
#endif
