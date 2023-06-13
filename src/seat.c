#include <wayland-server-core.h>
#include <wayland-util.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/util/log.h>
#include <stdlib.h>
#include "seat.h"
#include "server.h"
#include "view.h"

// cursor
void process_cursor_move(struct maple_server *server)
{
    struct maple_view *view = server->grabbed_view;

    wlr_scene_node_set_position(&view->scene_tree->node,
            server->cursor->x - server->grab_x,
            server->cursor->y - server->grab_y);
}

void process_cursor_resize(struct maple_server *server, uint32_t time)
{
    //TODO
}

static void process_cursor_motion(struct maple_server *server, uint32_t time)
{
    /* If the mode is non-passthrough, delegate to those functions. */
    if (server->cursor_mode == CURSOR_MOVE)
    {
        process_cursor_move(server);
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
    struct maple_view *view = desktop_view_at(server, server->cursor->x,
                        server->cursor->y, &surface, &sx, &sy);

    if (!view)
    {
        /* If there's no view under the cursor, set the cursor image to a
		 * default. This is what makes the cursor image appear when you move it
		 * around the screen, not over any views. */
        wlr_xcursor_manager_set_cursor_image(server->cursor_mngr,
                        server->cursor_mngr->name, server->cursor);
    }

    if (surface)
    {
        /*
		 * Send pointer enter and motion events.
		 *
		 * The enter event gives the surface "pointer focus", which is distinct
		 * from keyboard focus. You get pointer focus by moving the pointer over
		 * a window.
		 *
		 * Note that wlroots will avoid sending duplicate enter/motion events if
		 * the surface has already has pointer focus or if the client is already
		 * aware of the coordinates passed.
		 */

        wlr_seat_pointer_notify_enter(seat, surface, sx, sy);
        wlr_seat_pointer_notify_motion(seat, time, sx, sy);
    }
    else
    {
        /* Clear pointer focus so future button events and such are not sent to
		 * the last client to have the cursor over it. */
        wlr_seat_pointer_clear_focus(seat);
    }
}

static void server_cursor_motion(struct wl_listener *listener, void *data)
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
static void server_cursor_motion_absolute(struct wl_listener *listener, void *data)
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

static void reset_cursor_mode(struct maple_server *server)
{
    /* Reset the cursor mode to passthrough. */
    server->cursor_mode = CURSOR_PASSTHROUGH;
    server->grabbed_view = nullptr;
}

static void server_cursor_button(struct wl_listener *listener, void *data)
{
    /* This event is forwarded by the cursor when a pointer emits a button
	 * event. */

    struct maple_server *server = wl_container_of(listener, server, cursor_button);

    struct wlr_pointer_button_event *event = data;

    /* Notify the client with pointer focus that a button press has occurred */
    wlr_seat_pointer_notify_button(server->seat,
            event->time_msec, event->button, event->state);

    double sx, sy;
    struct wlr_surface *surface = nullptr;
    struct maple_view *view = desktop_view_at(server,
                server->cursor->x, server->cursor->y, &surface, &sx, &sy);

    if (event->state == WLR_BUTTON_RELEASED)
    {
        /* If you released any buttons, we exit interactive move/resize mode. */
        reset_cursor_mode(server);
    }
    else
    {
        /* Focus that client if the button was _pressed_ */
        focus_view(view, surface);
    }
}

static void server_cursor_axis(struct wl_listener *listener, void *data)
{
    /* This event is forwarded by the cursor when a pointer emits an axis event,
	 * for example when you move the scroll wheel. */

    struct maple_server *server = wl_container_of(listener, server, cursor_axis);

    struct wlr_pointer_axis_event *event = data;

    /* Notify the client with pointer focus of the axis event. */
    wlr_seat_pointer_notify_axis(server->seat,
                event->time_msec, event->orientation,event->delta,
                event->delta_discrete,event->source);
}

static void server_cursor_frame (struct wl_listener *listener, void *data)
{
    /* This event is forwarded by the cursor when a pointer emits a frame
	 * event. Frame events are sent after regular pointer events to group
	 * multiple events together. For instance, two axis events may happen at the
	 * same time, in which case a frame event won't be sent in between. */
    struct maple_server *server = wl_container_of(listener, server, cursor_frame);
    (void) data;
    /* Notify the client with pointer focus of the frame event. */
	wlr_seat_pointer_notify_frame(server->seat);

}


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
    server->cursor_motion.notify = server_cursor_motion;
    wl_signal_add(&server->cursor->events.motion, &server->cursor_motion);

    server->cursor_motion_absolute.notify = server_cursor_motion_absolute;
    wl_signal_add(&server->cursor->events.motion_absolute, &server->cursor_motion_absolute);

    server->cursor_button.notify = server_cursor_button;
    wl_signal_add(&server->cursor->events.button, &server->cursor_button);

    server->cursor_axis.notify = server_cursor_axis;
    wl_signal_add(&server->cursor->events.axis, &server->cursor_axis);

    server->cursor_frame.notify = server_cursor_frame;
    wl_signal_add(&server->cursor->events.frame, &server->cursor_frame);

    return true;
}
// cursor


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

    server->new_input.notify = new_input;
    wl_signal_add(&server->backend->events.new_input, &server->new_input);
    server->seat = wlr_seat_create(server->wl_display, "seat0");

    server->request_cursor.notify = seat_request_cursor;
    wl_signal_add(&server->seat->events.request_set_cursor, &server->request_cursor);

    server->request_set_selection.notify = seat_request_set_selection;
    wl_signal_add(&server->seat->events.request_set_selection, &server->request_set_selection);



    return true;
}
