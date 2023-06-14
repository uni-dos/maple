#ifndef MAPLE_SERVER_H
#define MAPLE_SERVER_H


#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/xwayland.h>


struct maple_view;

enum maple_cursor_mode {
    CURSOR_PASSTHROUGH,
    CURSOR_MOVE,
    CURSOR_RESIZE,
};

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


    //views
    struct wl_list views;
    struct wlr_xdg_shell *xdg_shell;
    struct wl_listener new_xdg_surface;

    struct wlr_xwayland *xwayland;
    struct wl_listener xwayland_ready;
    struct wl_listener new_xwayland_surface;

    //stuff for the visible cursor
    enum maple_cursor_mode cursor_mode;
    struct wlr_cursor *cursor;
    struct wlr_xcursor_manager *cursor_mngr;

    struct wl_listener cursor_motion;
    struct wl_listener cursor_motion_absolute;

    // called when a mouse button was pressed
    struct wl_listener cursor_button;

   // called when the scroll wheel is activated
    struct wl_listener cursor_axis;

    struct wl_listener cursor_frame;
    //cursor

    struct wlr_seat *seat;

    struct wl_listener new_input;
    struct wl_listener request_cursor;
    struct wl_listener request_set_selection;

    struct wl_list keyboards;

    struct maple_view *grabbed_view;
    double grab_x, grab_y;
    struct wlr_box grab_geobox;
    uint32_t resize_edges;


    //list of all physical displays/what we can see
    struct wl_list outputs;
    struct wlr_output_layout *output_layout;
    struct wl_listener new_output;


};

#endif
