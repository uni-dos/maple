#ifndef MAPLE_XDG_SHELL_H
#define MAPLE_XDG_SHELL_H
#include <wayland-server-core.h>

struct maple_xdg_shell_view {
	struct wl_list link;

	struct tinywl_server *server;
	struct wlr_xdg_toplevel *xdg_toplevel;
	struct wlr_scene_tree *scene_tree;
	struct wl_listener map;
	struct wl_listener unmap;
	struct wl_listener destroy;
	struct wl_listener request_move;
	struct wl_listener request_resize;
	struct wl_listener request_maximize;
	struct wl_listener request_fullscreen;
};

#endif
