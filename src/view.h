#ifndef MAPLE_VIEW_H
#define MAPLE_VIEW_H


#include "server.h"
#include <wayland-util.h>

/* Window state */
struct maple_view {

    /* Number of windows */
    struct wl_list link;
    struct maple_server *server;

    struct wlr_xdg_toplevel *xdg_toplevel;
    struct wlr_scene_tree *scene_tree;
};

#endif
