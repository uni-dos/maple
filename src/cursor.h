#ifndef MAPLE_CURSOR_H
#define MAPLE_CURSOR_H

#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_xcursor_manager.h>

extern struct maple_server server;

// maybe used to change the cursor image?
enum maple_cursor_mode {
    CURSOR_PASSTHROUGH,
    CURSOR_MOVE,
    CURSOR_RESIZE,
};

// holds the state for the the seat
struct maple_cursor {
    struct maple_server *server;
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

struct maple_cursor* setup_cursor(struct maple_server *server);
#endif
