#ifndef MAPLE_VIEW_H
#define MAPLE_VIEW_H

#include <wayland-server-core.h>


enum maple_view_type {
  MAPLE_VIEW_XDG_SHELL,
  MAPLE_VIEW_XWAYLAND,

};

/* Window state
   Can either be xwayland or xdg-shell windows
   needs to be abstracted to allow both
*/
struct maple_view {

    enum maple_view_type view_type;
    struct wl_list link;

    struct maple_server *server;


    struct wlr_xdg_toplevel *xdg_top_level;
    struct wlr_scene_tree *scene_tree;

    struct wl_listener map;
    struct wl_listener unmap;
    struct wl_listener destroy;

    /*Change the actual window */
    struct wl_listener request_move;
    struct wl_listener request_resize;
    struct wl_listener request_maximize;
    struct wl_listener request_fullscreen;
};

#endif
