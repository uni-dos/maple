#include <wayland-server-core.h>
#include <wayland-util.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <stdlib.h>
#include "seat.h"
#include "server.h"
#include "view.h"


void process_cursor_move(struct maple_server *server, uint32_t time)
{

}

void process_cursor_resize(struct maple_server *server, uint32_t time)
{

}

static void process_cursor_motion(struct maple_server *server, uint32_t time)
{
    /* If the mode is non-passthrough, delegate to those functions. */
    if (server->cursor_mode == CURSOR_MOVE)
    {
        process_cursor_move(server, time);
        return;
    }
    else if (server->cursor_mode == CURSOR_RESIZE)
    {
        process_cursor_resize(server, time);
        return;
    }

    /* Otherwise, find the view under the pointer and send the event along. */
    double sx, sy;

    struct wlr_seat *seat = server->seat;
    struct wlr_surface *surface = nullptr;
    struct maple_view *view = nullptr;

    desktop_view_at(server, server->cursor->x,
                        server->cursor->y, &surface, &sx, &sy);

    if (!view)
    {
        /* If there's no view under the cursor, set the cursor image to a
		 * default. This is what makes the cursor image appear when you move it
		 * around the screen, not over any views. */
        wlr_xcursor_manager_set_cursor_image(server->cursor_mngr,
                        server->cursor_mngr->name, server->cursor);
    }
}

static void cursor_motion(struct wl_listener *listener, void *data)
{
    /* This event is forwarded by the cursor when a pointer emits a _relative_
     * pointer motion event (i.e. a delta) */

    struct maple_server *server = wl_container_of(listener, server , cursor_motion);

    struct wlr_pointer_motion_event *event = data;

    /* The cursor doesn't move unless we tell it to. The cursor automatically
	 * handles constraining the motion to the output layout, as well as any
	 * special configuration applied for the specific input device which
	 * generated the event. You can pass NULL for the device if you want to move
	 * the cursor around without any input. */

    wlr_cursor_move(server->cursor,&event->pointer->base, event->delta_x, event->delta_y);
    process_cursor_motion(server, event->time_msec);
}

// runs when this is embedded
static void cursor_motion_absolute(struct wl_listener *listener, void *data)
{
    /* This event is forwarded by the cursor when a pointer emits an _absolute_
	 * motion event, from 0..1 on each axis. This happens, for example, when
	 * wlroots is running under a Wayland window rather than KMS+DRM, and you
	 * move the mouse over the window. You could enter the window from any edge,
	 * so we have to warp the mouse there. There is also some hardware which
	 * emits these events. */

    struct maple_server *server = wl_container_of(listener, server, cursor_motion_absolute);

    struct wlr_pointer_motion_absolute_event *event = data;

    wlr_cursor_warp_absolute(server->cursor, &event->pointer->base, event->x, event->y);
    process_cursor_motion(server, event->time_msec);
}

static void cursor_button(struct wl_listener *listener, void *data) {}

static void cursor_axis(struct wl_listener *listener, void *data) {}

static void cursor_frame (struct wl_listener *listener, void *data) {}


static bool setup_cursor(struct maple_server *server) {

    /* Creates a cursor to track on screen */
    server->cursor = wlr_cursor_create();
    if (!server->cursor) {
        return false;
    }
    server->cursor_mode = CURSOR_PASSTHROUGH;

    char *xcursor_theme = getenv("XCURSOR_THEME");
    char *xcursor_size = getenv("XCURSOR_SIZE");

    //if XCURSOR_SIZE was not set, make it 24
    unsigned int size = xcursor_size ? strtoul(xcursor_size, nullptr, 10) : 24;

    /* Creates an xcursor manager, another wlroots utility which loads up
    * Xcursor themes to source cursor images from and makes sure that cursor
    * images are available at all scale factors on the screen (necessary for
    * HiDPI support). We add a cursor theme at scale factor 1 to begin with. */
    server->cursor_mngr = wlr_xcursor_manager_create(xcursor_theme, size);


    wlr_xcursor_manager_load(server->cursor_mngr, 1);

    /*
     * wlr_cursor *only* displays an image on screen. It does not move around
     * when the pointer moves. However, we can attach input devices to it, and
     * it will generate aggregate events for all of them. In these events, we
     * can choose how we want to process them, forwarding them to clients and
     * moving the cursor around. More detail on this process is described in my
     * input handling blog post:
     *
     * https://drewdevault.com/2018/07/17/Input-handling-in-wlroots.html
     *
     */

    server->cursor_mode = CURSOR_PASSTHROUGH;
    server->cursor_motion.notify = cursor_motion;
    wl_signal_add(&server->cursor->events.motion, &server->cursor_motion);

    server->cursor_motion_absolute.notify = cursor_motion_absolute;
    wl_signal_add(&server->cursor->events.motion_absolute, &server->cursor_motion_absolute);

    server->cursor_button.notify = cursor_button;
    wl_signal_add(&server->cursor->events.button, &server->cursor_button);

    server->cursor_axis.notify = cursor_axis;
    wl_signal_add(&server->cursor->events.axis, &server->cursor_axis);

    server->cursor_frame.notify = cursor_frame;
    wl_signal_add(&server->cursor->events.frame, &server->cursor_frame);

    return true;
}



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



bool setup_seat(struct maple_server *server)
{
    bool cursor_setup = setup_cursor(server);

    if (!cursor_setup) {
        return false;
    }


    server->new_input.notify = new_input;
    wl_signal_add(&server->backend->events.new_input, &server->new_input);

    return true;
}
