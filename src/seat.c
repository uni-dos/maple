#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_input_device.h>
#include "seat.h"
#include "cursor.h"

static void new_pointer(struct maple_server *server, struct wlr_input_device *device) {

    /* We don't do anything special with pointers. All of our pointer handling
	 * is proxied through wlr_cursor. On another compositor, you might take this
	 * opportunity to do libinput configuration on the device to set
	 * acceleration, etc. */

    wlr_cursor_attach_input_device(server->cursor, device);
}

static void new_input(struct wl_listener *listener, void *data)
{
    struct maple_server *server = wl_container_of(listener, server, new_input);
    struct wlr_input_device *device = data;

    switch(device->type) {
        case WLR_INPUT_DEVICE_KEYBOARD:
        //setup keyboard
            break;
        case WLR_INPUT_DEVICE_POINTER:
            new_pointer(server, device);
            break;

        default:
            break;
    }
}



bool setup_seat(struct maple_server *server)
{
    bool cursor_setup = setup_cursor(server);

    if (!cursor_setup) {
        return false;
    }


    server->new_input.notify = new_input;

    return true;
}
