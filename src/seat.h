#ifndef MAPLE_SEAT_H
#define MAPLE_SEAT_H
#include <wlr/types/wlr_keyboard.h>
#include "server.h"

struct maple_keyboard {

    struct wl_list link;
    struct maple_server *server;
    struct wlr_keyboard *wlr_keyboard;

    struct wl_listener modifiers;
    struct wl_listener key;
    struct wl_listener destroy;

};

bool setup_seat(struct maple_server *server);

#endif
