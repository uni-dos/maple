#include <wlr/types/wlr_seat.h>
#include "view.h"
/*
    view.c is heavily inspired from stage
    mostly the xwayland implementation
*/

static bool is_view_x11(struct maple_view *view)
{
    return view->view_type == VIEW_X11;
}

static void xdg_toplevel_unmap(struct wl_listener *listener, void *data)
{

}

static void xdg_toplevel_map(struct wl_listener *listener, void *data)
{

}

static void xdg_toplevel_destroy(struct wl_listener *listener, void *data)
{

}

static void server_new_xdg_surface(struct wl_listener *listener, void *data)
{
    /* This event is raised when wlr_xdg_shell receives a new xdg surface from a
	 * client, either a toplevel (application window) or popup. */
    struct maple_server *server = wl_container_of(listener, server, new_xdg_surface);

    struct wlr_xdg_surface *xdg_surface = data;

    /* We must add xdg popups to the scene graph so they get rendered. The
	 * wlroots scene graph provides a helper for this, but to use it we must
	 * provide the proper parent scene node of the xdg popup. To enable this,
	 * we always set the user data field of xdg_surfaces to the corresponding
	 * scene node. */
    if (xdg_surface->role == WLR_XDG_SURFACE_ROLE_POPUP)
    {
        struct wlr_xdg_surface *parent = wlr_xdg_surface_try_from_wlr_surface(xdg_surface->popup->parent);
        if (parent == nullptr)
            return;

        struct wlr_scene_tree *parent_tree = parent->data;

        xdg_surface->data = wlr_scene_xdg_surface_create(parent_tree, xdg_surface);
        return;
    }

    if (xdg_surface->role != WLR_XDG_SURFACE_ROLE_TOPLEVEL)
        return;

    /* Allocate a maple_view for this surface
       is manually created
     */
     struct maple_view *view = calloc(1, sizeof(struct maple_view));

     view->server = server;
     view->view_type = VIEW_XDG;
     view->xdg_toplevel = xdg_surface->toplevel;
     view->scene_tree = wlr_scene_xdg_surface_create(
            &view->server->scene->tree, view->xdg_toplevel->base);
    view->scene_tree->node.data = view;
    xdg_surface->data = view->scene_tree;

    /* Listen to the various events it can emit */
	view->map.notify = xdg_toplevel_map;
	wl_signal_add(&xdg_surface->events.map, &view->map);
	view->unmap.notify = xdg_toplevel_unmap;
	wl_signal_add(&xdg_surface->events.unmap, &view->unmap);
	view->destroy.notify = xdg_toplevel_destroy;
	wl_signal_add(&xdg_surface->events.destroy, &view->destroy);

	/* will repeat later for xwayland */
	struct wlr_xdg_toplevel *toplevel = xdg_surface->toplevel;
	view->request_move.notify = xdg_toplevel_request_move;
	wl_signal_add(&toplevel->events.request_move, &view->request_move);
	view->request_resize.notify = xdg_toplevel_request_resize;
	wl_signal_add(&toplevel->events.request_resize, &view->request_resize);
	view->request_maximize.notify = xdg_toplevel_request_maximize;
	wl_signal_add(&toplevel->events.request_maximize,
		&view->request_maximize);
	view->request_fullscreen.notify = xdg_toplevel_request_fullscreen;
	wl_signal_add(&toplevel->events.request_fullscreen,
		&view->request_fullscreen);


}

static void server_xwayland_ready(struct wl_listener *listener, void * data)
{
    (void) data;
    struct maple_server *server = wl_container_of(listener, server, xwayland_ready);

    wlr_xwayland_set_seat(server->xwayland, server->seat);
}

static void server_new_xwayland_surface(struct wl_listener *listener, void * data)
{
    struct maple_server *server = wl_container_of(listener, server, new_xwayland_surface);

    struct wlr_xwayland_surface *xwayland_surface = data;

    struct maple_view *view = calloc(1, sizeof(struct maple_view));

    view->server = server;
    view->view_type = VIEW_X11;
    view->xwayland_surface = xwayland_surface;

    view->map.notify = xdg_toplevel_map;
	wl_signal_add(&xwayland_surface->events.map, &view->map);

	view->unmap.notify = xdg_toplevel_unmap;
	wl_signal_add(&xwayland_surface->events.unmap, &view->unmap);

	view->destroy.notify = xdg_toplevel_destroy;
	wl_signal_add(&xwayland_surface->events.destroy, &view->destroy);

	view->activate.notify = activate;
	wl_signal_add(&xwayland_surface->events.request_activate,
	    &view->activate);

	view->configure.notify = configure;
	wl_signal_add(&xwayland_surface->events.request_configure,
	    &view->configure);


}

bool setup_views(struct maple_server *server)
{
     server->new_xdg_surface.notify = server_new_xdg_surface;
	 wl_signal_add(&server->xdg_shell->events.new_surface,
	 		&server->new_xdg_surface);

    if (server->xwayland)
    {
        server->xwayland_ready.notify = server_xwayland_ready;
        wl_signal_add(&server->xwayland->events.ready, &server->xwayland_ready);


        server->new_xwayland_surface.notify = server_new_xwayland_surface;
        wl_signal_add(&server->xwayland->events.new_surface, &server->new_xwayland_surface);

        setenv("DISPLAY", server->xwayland->display_name, true);

    }
    return true;
}

struct maple_view* desktop_view_at(struct maple_server *server,double lx, double ly,
            struct wlr_surface **surface, double *sx, double *sy)
{
    /* This returns the topmost node in the scene at the given layout coords.
    * we only care about surface nodes as we are specifically looking for a
    * surface in the surface tree of a tinywl_view. */

    struct wlr_scene_node *node = wlr_scene_node_at(
        &server->scene->tree.node, lx, ly, sx, sy);

    if(node == nullptr || node->type != WLR_SCENE_NODE_BUFFER)
        return nullptr;

    struct wlr_scene_buffer *scene_buffer = wlr_scene_buffer_from_node(node);
    struct wlr_scene_surface *scene_surface = wlr_scene_surface_try_from_buffer(scene_buffer);

    if (!scene_surface)
        return nullptr;

    *surface = scene_surface->surface;

    /* Find the node corresponding to the tinywl_view at the root of this
	 * surface tree, it is the only one for which we set the data field. */
    struct wlr_scene_tree *tree = node->parent;

    while (tree != nullptr && tree->node.data == nullptr)
    {
        tree = tree->node.parent;
    }

    return tree->node.data;

}

struct wlr_seat;

void focus_view(struct maple_view *view, struct wlr_surface *surface)
{
    /* Note: this function only deals with keyboard focus. */
    if (view == nullptr)
        return;

    struct maple_server *server = view->server;

    struct wlr_seat *seat = server->seat;

    struct wlr_surface *prev_surface = seat->keyboard_state.focused_surface;

    if (prev_surface == surface)
    {
        /* Don't re-focus an already focused surface. */
        return;
    }
    if (prev_surface)
    {
        /*
		 * Deactivate the previously focused surface. This lets the client know
		 * it no longer has focus and the client will repaint accordingly, e.g.
		 * stop displaying a caret.
		 */


        //TODO need to implement xdg vs xwayland logic
    }

}
