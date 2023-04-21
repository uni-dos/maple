#ifndef MAPLE_SERVER_H
#define MAPLE_SERVER_H

#include "view.h"
#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/xwayland.h>


// everything needed to be a wayland compositor
// the server is the wm; holds its state
struct maple_server {

    struct wl_display *wl_display;

    struct wlr_backend *backend;


    struct wlr_renderer *renderer;
    struct wlr_allocator *allocator;
    struct wlr_scene *scene;

    struct wlr_compositor *compositor;
    struct wlr_subcompositor *subcompositor;

    struct wlr_data_device_manager *data_device_manager;

    struct wl_list views;
    struct wlr_xdg_shell *xdg_shell;
    struct wl_listener new_xdg_surface;

    struct wlr_xwayland *xwayland;
    struct wl_listener new_xwayland_surface;

    struct maple_cursor *cursor;
    struct wlr_seat *seat;

    //list of all physical displays/what we can see
    struct wl_list outputs;
    struct wlr_output_layout *output_layout;
    struct wl_listener new_output;


};

#endif
