#include <stdlib.h>
#include <wayland-util.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/xwayland.h>
#include <wlr/util/log.h>

#include "output.h"
#include "seat.h"
#include "server.h"
#include "view.h"


static void server_destroy(struct maple_server *server)
{
    /* Once wl_display_run returns, we shut down the server. */
    wl_display_destroy_clients(server->wl_display);
    wlr_scene_node_destroy(&server->scene->tree.node);
    wlr_xcursor_manager_destroy(server->cursor_mngr);
    wlr_output_layout_destroy(server->output_layout);
    wl_display_destroy(server->wl_display);
}

static bool start_server(struct maple_server *server)
{
     const char *socket = wl_display_add_socket_auto(server->wl_display);

    if (!socket)
    {
        wlr_log(WLR_ERROR, "Failed to setup socket");
        wlr_backend_destroy(server->backend);
        return false;
    }

    /* Start the backend. This will enumerate outputs and inputs, become the DRM
    * master, etc */
    if (!wlr_backend_start(server->backend))
    {
        wlr_log(WLR_ERROR, "Failed to start backend");
        wlr_backend_destroy(server->backend);
        wl_display_destroy(server->wl_display);
        return false;
    }

    /* Set the WAYLAND_DISPLAY environment variable to our socket */
    setenv("WAYLAND_DISPLAY", socket, true);
    wlr_log(WLR_INFO, "Running maple on WAYLAND_DISPLAY=%s", socket);
    return true;
}

static bool server_init(struct maple_server *server)
{
    // If statements added to pinpoit possible failures.

    /* The Wayland display is managed by libwayland. It handles accepting
     * clients from the Unix socket, manging Wayland globals, and so on. */
    server->wl_display = wl_display_create();
    if (server->wl_display == nullptr)
    {
        wlr_log(WLR_ERROR, "Failed to create to Wayland display");
        return false;
    }

    /* The backend is a wlroots feature which abstracts the underlying input and
    * output hardware. The autocreate option will choose the most suitable
    * backend based on the current environment, either a window on Wayland/X or
    * physical hardware.*/
    server->backend = wlr_backend_autocreate(server->wl_display, nullptr);
    if (server->backend == nullptr)
    {
        wlr_log(WLR_ERROR, "Failed to create backend");
        return false;
    }

    /* Autocreates a renderer, either Pixman, GLES2 or Vulkan for us. The user
    * can also specify a renderer using the WLR_RENDERER env var.
    * The renderer is responsible for defining the various pixel formats it
    * supports for shared memory, this configures that for clients.*/
    server->renderer = wlr_renderer_autocreate(server->backend);

    if (server->renderer == nullptr)
    {
        wlr_log(WLR_ERROR, "Failed to create renderer");
        return false;
    }

    bool wl_disp_initilized = wlr_renderer_init_wl_display(server->renderer, server->wl_display);
    if (!wl_disp_initilized)
    {
        wlr_log(WLR_ERROR, "Failed to initilize wl_display");
        return false;
    }

    /* Autocreates an allocator for us.
    * The allocator is the bridge between the renderer and the backend. It
    * handles the buffer creation, allowing wlroots to render onto the
    * screen */
    server->allocator = wlr_allocator_autocreate(server->backend, server->renderer);
    if (server->allocator == nullptr)
    {
        wlr_log(WLR_ERROR, "Failed to create allocator");
        return false;
    }
    /* This creates some hands-off wlroots interfaces. The compositor is
    * necessary for clients to allocate surfaces, the subcompositor allows to
    * assign the role of subsurfaces to surfaces and the data device manager
    * handles the clipboard. Each of these wlroots interfaces has room for you
    * to dig your fingers in and play with their behavior if you want. Note that
    * the clients cannot set the selection directly without compositor approval,
    * see the handling of the request_set_selection event below.*/
    server->compositor = wlr_compositor_create(server->wl_display, 5, server->renderer);
    server->subcompositor= wlr_subcompositor_create(server->wl_display);
    server->data_device_manager = wlr_data_device_manager_create(server->wl_display);


    if(!set_up_output(server))
    {
        wlr_log(WLR_ERROR, "Failed to initilize output");
        return false;
    }

    /* Create a scene graph. This is a wlroots abstraction that handles all
     * rendering and damage tracking. All the compositor author needs to do
     * is add things that should be rendered to the scene graph at the proper
     * positions and then call wlr_scene_output_commit() to render a frame if
     * necessary. */
    server->scene = wlr_scene_create();
    if(!wlr_scene_attach_output_layout(server->scene, server->output_layout)) {
        wlr_log(WLR_ERROR, "Failed to attach output to scene graph");
    }

    //views

    wl_list_init(&server->views);
    server->xdg_shell = wlr_xdg_shell_create(server->wl_display, 5);

    server->xwayland = wlr_xwayland_create(server->wl_display, server->compositor, true);
    setup_views(server);

    // seat will set up the cursor, pointer device, and keyboard
    if (!setup_seat(server))
    {
        wlr_log(WLR_ERROR, "Failed to setup the seat");
        return false;
    }

    return true;
}

int main(int argc, char** argv)
{
    (void) argc;
    (void) argv;

    struct maple_server server = {0};

    if (!server_init(&server))
    {
        wlr_log(WLR_ERROR, "Server failed to initilize");
        return 1;
    }

    if(!start_server(&server))
    {
        wlr_log(WLR_ERROR, "Failed to start server");
        server_destroy(&server);
        return 1;
    }
    /* Run the Wayland event loop. This does not return until you exit the
	 * compositor. Starting the backend rigged up all of the necessary event
	 * loop configuration to listen to libinput events, DRM events, generate
	 * frame events at the refresh rate, and so on. */
    wl_display_run(server.wl_display);

    server_destroy(&server);

    return 0;
}


