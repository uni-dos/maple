#include "xdg.h"
#include "view.h"
#include "cursor.h"
static void begin_interactive(struct maple_view *view,
                    enum maple_cursor_mode mode, uint32_t edges)
{

}

static void xdg_toplevel_request_move(struct wl_listener *listener, void *data)
{

}

static void xdg_toplevel_request_resize(struct wl_listener *listener, void *data)
{

}


static void xdg_toplevel_request_maximize(struct wl_listener *listener, void *data)
{

}

static void xdg_toplevel_request_fullscreen(struct wl_listener *listener, void *data)
{

}


static void xdg_toplevel_unmap(struct wl_listener *listener, void *data)
{
    (void) data;
    /* Called when the surface is unmapped, and should no longer be shown. */
    struct maple_view *view = wl_container_of(listener, view, unmap);

    /* Reset the cursor mode if the grabbed view was unmapped. */
    if (view == view->server->grabbed_view)
    {
        reset_cursor_mode(view->server);
    }

    wl_list_remove(&view->link);
}

static void xdg_toplevel_map(struct wl_listener *listener, void *data)
{
    (void) data;
    /* Called when the surface is mapped, or ready to display on-screen. */
    struct maple_view *view = wl_container_of(listener, view, map);

    wl_list_insert(&view->server->views, &view->link);

    focus_view(view, view->xdg_toplevel->base->surface);
}

static void xdg_toplevel_destroy(struct wl_listener *listener, void *data)
{
    (void) data;
    /* Called when the surface is destroyed and should never be shown again. */

    struct maple_view *view = wl_container_of(listener, view, destroy);

    wl_list_remove(&view->map.link);
	wl_list_remove(&view->unmap.link);
	wl_list_remove(&view->destroy.link);
	wl_list_remove(&view->request_move.link);
	wl_list_remove(&view->request_resize.link);
	wl_list_remove(&view->request_maximize.link);
	wl_list_remove(&view->request_fullscreen.link);

    free(view);
}

void server_new_xdg_surface(struct wl_listener *listener, void *data)
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
	wl_signal_add(&xdg_surface->surface->events.map, &view->map);
	view->unmap.notify = xdg_toplevel_unmap;
	wl_signal_add(&xdg_surface->surface->events.unmap, &view->unmap);
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
