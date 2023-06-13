#ifndef MAPLE_VIEW_H
#define MAPLE_VIEW_H

#include <wayland-server-core.h>
#include <wlr/types/wlr_scene.h>
#include "server.h"

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


    struct wlr_surface *surface; // NULL for unmapped views
    struct wlr_xdg_surface *xdg_surface;
    struct wlr_xwayland_surface *xwayland_surface;

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

struct maple_view* desktop_view_at(struct maple_server *server,double x,
              double y, struct wlr_surface **surface, double *sx, double *sy);

void focus_view(struct maple_view *view, struct wlr_surface *surface);

bool setup_views(struct maple_server *server);
#endif
