#ifndef MAPLE_XWAYLAND_H
#define MAPLE_XWAYLAND_H
#include "wlr/xwayland/xwayland.h"

void server_xwayland_ready(struct wl_listener *listener, void * data);
void server_new_xwayland_surface(struct wl_listener *listener, void * data);
#endif
