#ifndef MAPLE_KEYBOARD_H
#define MAPLE_KEYBOARD_H
#include <wayland-server-core.h>

struct maple_keyboard {

    struct wl_list link;
    struct maple_server *server;
    struct wlr_keyboard *wlr_keyboard;

    struct wl_listener modifiers;
    struct wl_listener key;
    struct wl_listener destroy;

};

#endif
