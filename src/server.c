#include <stdlib.h>
#include <wayland-server-core.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/util/log.h>
#include "output.h"
#include "server.h"

static void setup_socket(struct maple_server *server)
{
     const char *socket = wl_display_add_socket_auto(server->wl_display);

    if (!socket)
    {
        wlr_backend_destroy(server->backend);
        return;
    }

    /* Start the backend. This will enumerate outputs and inputs, become the DRM
    * master, etc */
    if (!wlr_backend_start(server->backend))
    {
        wlr_log(WLR_ERROR, "Failed to start backend");
        wlr_backend_destroy(server->backend);
        wl_display_destroy(server->wl_display);
        return;
    }

    setenv("WAYLAND_DISPLAY", socket, true);
}

bool server_init(struct maple_server *server)
{
    // If statements added to pinpoit possible failures.

    /* The Wayland display is managed by libwayland. It handles accepting
     * clients from the Unix socket, manging Wayland globals, and so on. */
    server->wl_display = wl_display_create();
    if (server->wl_display == NULL)
    {
        wlr_log(WLR_ERROR, "Failed to create to Wayland display");
        return false;
    }

    /* The backend is a wlroots feature which abstracts the underlying input and
    * output hardware. The autocreate option will choose the most suitable
    * backend based on the current environment, either a window on Wayland/X or
    * physical hardware.*/
    server->backend = wlr_backend_autocreate(server->wl_display);
    if (server->backend == NULL)
    {
        wlr_log(WLR_ERROR, "Failed to create backend");
        return false;
    }

    /* Autocreates a renderer, either Pixman, GLES2 or Vulkan for us. The user
    * can also specify a renderer using the WLR_RENDERER env var.
    * The renderer is responsible for defining the various pixel formats it
    * supports for shared memory, this configures that for clients.*/
    server->renderer = wlr_renderer_autocreate(server->backend);

    if (server->renderer == NULL)
    {
        wlr_log(WLR_ERROR, "Failed to create renderer");
        return false;
    }

    wlr_renderer_init_wl_display(server->renderer, server->wl_display);

    /* Autocreates an allocator for us.
    * The allocator is the bridge between the renderer and the backend. It
    * handles the buffer creation, allowing wlroots to render onto the
    * screen */
    server->allocator = wlr_allocator_autocreate(server->backend, server->renderer);

    /* This creates some hands-off wlroots interfaces. The compositor is
    * necessary for clients to allocate surfaces, the subcompositor allows to
    * assign the role of subsurfaces to surfaces and the data device manager
    * handles the clipboard. Each of these wlroots interfaces has room for you
    * to dig your fingers in and play with their behavior if you want. Note that
    * the clients cannot set the selection directly without compositor approval,
    * see the handling of the request_set_selection event below.*/
    wlr_compositor_create(server->wl_display, server->renderer);
    wlr_subcompositor_create(server->wl_display);
    wlr_data_device_manager_create(server->wl_display);


    set_up_output(server);

    /* Create a scene graph. This is a wlroots abstraction that handles all
    rendering and damage tracking. All the compositor author needs to do
    is add things that should be rendered to the scene graph at the proper
    positions and then call wlr_scene_output_commit() to render a frame if
    necessary. */
    server->scene = wlr_scene_create();
    if(!wlr_scene_attach_output_layout(server->scene, server->output_layout)) {
        wlr_log(WLR_ERROR, "Failed to attach output to scene graph");
    }

    setup_cursor(server);
    
    return true;
}

void server_run(struct maple_server *server)
{
    setup_socket(server);
    wl_display_run(server->wl_display);
}

void server_destroy(struct maple_server *server)
{

    wl_display_destroy_clients(server->wl_display);
    wl_display_destroy(server->wl_display);
}