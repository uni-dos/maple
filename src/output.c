#include <stdlib.h>
#include <time.h>
#include <wayland-server-core.h>
#include <wayland-util.h>
#include <wlr/util/log.h>
#include <wlr/types/wlr_output_layout.h>
#include "output.h"

/* Called every time the output is ready to display a frame */
static void server_output_frame(struct wl_listener *listener, void *data) {
    // not doing anyhting with this
    (void) data;

    struct maple_output *output = wl_container_of(listener, output, frame);
    struct wlr_scene *scene = output->server->scene;
    struct wlr_scene_output *scene_output = wlr_scene_get_scene_output(scene, output->wlr_output);

    wlr_scene_output_commit(scene_output);

    struct timespec now;

    clock_gettime(CLOCK_MONOTONIC, &now);
    wlr_scene_output_send_frame_done(scene_output, &now);

}

// called when maple is nested in another wm
static void server_output_request_state(struct wl_listener *listener, void *data)
{
    /* This function is called when the backend requests a new state for
	 * the output. For example, Wayland and X11 backends request a new mode
	 * when the output window is resized. */
    struct maple_output *output = wl_container_of(listener, output, request_state);
    const struct wlr_output_event_request_state *event = data;
    wlr_output_commit_state(output->wlr_output, event->state);
}


static void server_output_destroy(struct wl_listener * listener, void *data) {
    // not doing anything with the data
    (void) data;

    struct maple_output *output = wl_container_of(listener, output, destroy);
    wl_list_remove(&output->frame.link);
    wl_list_remove(&output->destroy.link);
    wl_list_remove(&output->link);

    free(output);
}

/* Called each time a new display becomes available */
static void server_new_output(struct wl_listener *listener, void *data) {
    struct maple_server *server = wl_container_of(listener, server, new_output);
    struct wlr_output *wlr_output = data;

    /* Configures the output created by the backend to use our allocator
    * and our renderer. Must be done once, before commiting the output */
    if(!wlr_output_init_render(wlr_output, server->allocator, server->renderer))
    {
        wlr_log(WLR_ERROR, "Failed to initilize output renderer");
        return;
    }

    /* Some backends don't have modes. DRM+KMS does, and we need to set a mode
    * before we can use the output. The mode is a tuple of (width, height,
    * refresh rate), and each monitor supports only a specific set of modes. We
    * just pick the monitor's preferred mode, a more sophisticated compositor
    * would let the user configure it. */
    if (!wl_list_empty(&wlr_output->modes)) {

        struct wlr_output_mode *mode = wlr_output_preferred_mode(wlr_output);
        wlr_output_set_mode(wlr_output, mode);
        wlr_output_enable(wlr_output, true);
        if(!wlr_output_commit(wlr_output))
        {
            return;
        }
    }

    /* Allocates and configures our state for this output */
    struct maple_output *output = calloc(1, sizeof(struct maple_output));
    output->server = server;
    output->wlr_output = wlr_output;

    /* Sets up a listener for the frame event */
    output->frame.notify = server_output_frame;
    wl_signal_add(&wlr_output->events.frame, &output->frame);

    /* Sets up a listener for the state request event. */
	output->request_state.notify = server_output_request_state;
	wl_signal_add(&wlr_output->events.request_state, &output->request_state);

    /* Sets up a listener for the destroy event */
    output->destroy.notify = server_output_destroy;
    wl_signal_add(&wlr_output->events.destroy, &output->destroy);

    wl_list_insert(&server->outputs, &output->link);


    /* Adds this to the output layout. The add_auto function arranges outputs
    * from left-to-right in the order they appear. A more sophisticated
    * compositor would let the user configure the arrangement of outputs in the
    * layout.
    *
    * The output layout utility automatically adds a wl_output global to the
    * display, which Wayland clients can see to find out information about the
    * output (such as DPI, scale factor, manufacturer, etc).*/
    wlr_output_layout_add_auto(server->output_layout, wlr_output);
}

bool set_up_output(struct maple_server *server) {

    /* Creates an output layout, which is a wlroots utility for working with an
    * arrangement of screens in a physical layout. */
    server->output_layout = wlr_output_layout_create();

    if (!server->output_layout)
    {
        wlr_log(WLR_ERROR, "Failed to create an output layout");
        return false;
    }
    /*Configure a listener to be notified when new outputs are available
    * on the backend.
    */
    wl_list_init(&server->outputs);

    server->new_output.notify = server_new_output;
    wl_signal_add(&server->backend->events.new_output, &server->new_output);

    return true;
}
