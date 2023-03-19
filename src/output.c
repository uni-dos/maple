#include "output.h"

//called each time a new display is added
static void new_output_notify(struct wl_listener *listener, void *data) {
    struct maple_server *server = wl_container_of(listener, server, new_output);
    struct wlr_output *wlr_output = data;
}
