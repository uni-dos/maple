#ifndef MAPLE_SEAT_H
#define MAPLE_SEAT_H

#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_xcursor_manager.h>

#include "server.h"

// maybe used to change the cursor image?
enum maple_cursor_mode {
    CURSOR_PASSTHROUGH,
    CURSOR_MOVE,
    CURSOR_RESIZE,
};

// holds the state for the the seat
struct maple_seat {

    struct maple_server *server;
    struct wlr_seat *seat;

    struct wl_listener new_input;
    struct wl_listener request_cursor;
    struct wl_listener request_set_selection;
    struct wl_list keyboards;

    // focused view (window)
    struct maple_view *grabbed_view;
    double grab_x, grab_y;
    struct wlr_box grab_geobox;
    uint32_t resize_edges;

    // keyboard stuff
    struct wl_list link;
    struct wlr_keyboard *keyboard;

    struct wl_listener modifiers;
    struct wl_listener key;
    struct wl_listener destroy_keyboard;
    // cursor stuff
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

};

bool setup_seat(struct maple_server *server);
#endif
