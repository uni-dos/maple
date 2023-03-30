#ifndef MAPLE_SERVER_H
#define MAPLE_SERVER_H

#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_scene.h>
#include "cursor.h"

// everything needed to be a wayland compositor
// the server is the wm; holds its state
struct maple_server {

    struct wl_display *wl_display;

    struct wlr_backend *backend;


    struct wlr_renderer *renderer;
    struct wlr_allocator *allocator;
    struct wlr_scene *scene;

    //list of all physical displays/what we can see
    struct wl_list outputs;
    struct wlr_output_layout *output_layout;
    struct wl_listener new_output;

    struct maple_cursor *cursor;

};

// initilize most of the parameters in server
bool server_init(struct maple_server *server);

void server_run(struct maple_server *server);

void server_destroy(struct maple_server *server);


#endif
