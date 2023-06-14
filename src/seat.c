#include <wayland-server-core.h>
#include <wayland-util.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_input_device.h>

#include <wlr/util/log.h>
#include <stdlib.h>
#include "cursor.h"
#include "seat.h"
#include "server.h"
#include "view.h"

//attaches the pointer to the cursor
static void server_new_pointer(struct maple_server *server, struct wlr_input_device *device) {

    /* We don't do anything special with pointers. All of our pointer handling
	 * is proxied through wlr_cursor. On another compositor, you might take this
	 * opportunity to do libinput configuration on the device to set
	 * acceleration, etc. */

    wlr_cursor_attach_input_device(server->cursor, device);
}

void keyboard_handle_modifiers(struct wl_listener *listener, void *data)
{

}

void keyboard_handle_key(struct wl_listener *listener, void *data)
{

}

void keyboard_handle_destroy(struct wl_listener *listener, void *data)
{

}

static void server_new_keyboard(struct maple_server *server, struct wlr_input_device *device)
{
    struct wlr_keyboard *wlr_keyboard = wlr_keyboard_from_input_device(device);

    struct maple_keyboard *keyboard = calloc(1, sizeof(struct maple_keyboard));

    keyboard->server = server;
    keyboard->wlr_keyboard = wlr_keyboard;

    /* We need to prepare an XKB keymap and assign it to the keyboard. This
	 * assumes the defaults (e.g. layout = "us"). */

    struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    struct xkb_keymap *key_map = xkb_keymap_new_from_names(context, nullptr,
            XKB_KEYMAP_COMPILE_NO_FLAGS);

    wlr_keyboard_set_keymap(keyboard->wlr_keyboard, key_map);

    xkb_keymap_unref(key_map);
    xkb_context_unref(context);

    wlr_keyboard_set_repeat_info(wlr_keyboard, 25, 600);


    /* Here we set up listeners for keyboard events. */
    //TODO implement these
    keyboard->modifiers.notify = keyboard_handle_modifiers;
	wl_signal_add(&wlr_keyboard->events.modifiers, &keyboard->modifiers);

	keyboard->key.notify = keyboard_handle_key;
	wl_signal_add(&wlr_keyboard->events.key, &keyboard->key);

	keyboard->destroy.notify = keyboard_handle_destroy;
	wl_signal_add(&device->events.destroy, &keyboard->destroy);

	wlr_seat_set_keyboard(server->seat, keyboard->wlr_keyboard);

	/* And add the keyboard to our list of keyboards */
	wl_list_insert(&server->keyboards, &keyboard->link);


}

static void server_new_input(struct wl_listener *listener, void *data)
{
    struct maple_server *server = wl_container_of(listener, server, new_input);
    struct wlr_input_device *device = data;

    switch(device->type) {
        case WLR_INPUT_DEVICE_KEYBOARD:
        //setup keyboard
            server_new_keyboard(server, device);
            break;
        case WLR_INPUT_DEVICE_POINTER:
            server_new_pointer(server, device);
            break;

        default:
            break;
    }

    /* We need to let the wlr_seat know what our capabilities are, which is
	 * communiciated to the client. We always have a cursor, even if
	 * there are no pointer devices, so we always include that capability. */
    uint32_t caps = WL_SEAT_CAPABILITY_POINTER;

    if (!wl_list_empty(&server->keyboards))
    {
        caps |= WL_SEAT_CAPABILITY_KEYBOARD;
    }

    wlr_seat_set_capabilities(server->seat, caps);
}

void seat_request_cursor(struct wl_listener *listener, void *data)
{
    struct maple_server *server = wl_container_of(listener, server, request_cursor);

    /* This event is raised by the seat when a client provides a cursor image */
    struct wlr_seat_pointer_request_set_cursor_event *event = data;

    struct wlr_seat_client *focused_client = server->seat->pointer_state.focused_client;

    /* This can be sent by any client, so we check to make sure this one is
	 * actually has pointer focus first. */
    if (focused_client == event->seat_client)
    {
        /* Once we've vetted the client, we can tell the cursor to use the
		 * provided surface as the cursor image. It will set the hardware cursor
		 * on the output that it's currently on and continue to do so as the
		 * cursor moves between outputs. */
        wlr_cursor_set_surface(server->cursor, event->surface,
                event->hotspot_x, event->hotspot_y);
    }
}

void seat_request_set_selection(struct wl_listener *listener, void *data)
{
    /* This event is raised by the seat when a client wants to set the selection,
	 * usually when the user copies something. wlroots allows compositors to
	 * ignore such requests if they so choose, but in tinywl we always honor
	 */
    struct maple_server *server = wl_container_of(listener, server, request_set_selection);

    struct wlr_seat_request_set_selection_event *event = data;

    wlr_seat_set_selection(server->seat, event->source, event->serial);
}

// starting point for this file
bool setup_seat(struct maple_server *server)
{
    bool cursor_setup = setup_cursor(server);

    if (!cursor_setup) {
        wlr_log(WLR_ERROR, "Could not setup the cursor");
        return false;
    }

    /*
	 * Configures a seat, which is a single "seat" at which a user sits and
	 * operates the computer. This conceptually includes up to one keyboard,
	 * pointer, touch, and drawing tablet device. We also rig up a listener to
	 * let us know when new input devices are available on the backend.
	 */

    wl_list_init(&server->keyboards);

    server->new_input.notify = server_new_input;
    wl_signal_add(&server->backend->events.new_input, &server->new_input);
    server->seat = wlr_seat_create(server->wl_display, "seat0");

    server->request_cursor.notify = seat_request_cursor;
    wl_signal_add(&server->seat->events.request_set_cursor, &server->request_cursor);

    server->request_set_selection.notify = seat_request_set_selection;
    wl_signal_add(&server->seat->events.request_set_selection, &server->request_set_selection);



    return true;
}
