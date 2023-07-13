#include <wlr/types/wlr_seat.h>
#include "view.h"
#include "cursor.h"
#include "xdg.h"
#include "xwayland.h"

/*
    view.c is heavily inspired from stage
    mostly the xwayland implementation
*/

static bool is_view_x11(struct maple_view *view)
{
    return view->view_type == VIEW_X11;
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

    /* Find the node corresponding to the maple_view at the root of this
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
        struct wlr_xdg_surface *previous = wlr_xdg_surface_try_from_wlr_surface(prev_surface);

        if (previous == nullptr || previous->role != WLR_XDG_SURFACE_ROLE_TOPLEVEL)
        {
            return;
        }

        wlr_xdg_toplevel_set_activated(previous->toplevel, false);

    }

    struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(seat);

    /* Move the view to the front */
    wlr_scene_node_raise_to_top(&view->scene_tree->node);
    wl_list_remove(&view->link);
    wl_list_insert(&server->views, &view->link);
    /* Activate the new surface */
    wlr_xdg_toplevel_set_activated(view->xdg_toplevel, true);

    /*
	 * Tell the seat to have the keyboard enter this surface. wlroots will keep
	 * track of this and automatically send key events to the appropriate
	 * clients without additional work on your part.
	 */

    if (keyboard != nullptr)
    {
        wlr_seat_keyboard_notify_enter(seat, view->xdg_toplevel->base->surface,
            keyboard->keycodes, keyboard->num_keycodes, &keyboard->modifiers);
    }
}

bool setup_views(struct maple_server *server)
{
     server->new_xdg_surface.notify = server_new_xdg_surface;
	 wl_signal_add(&server->xdg_shell->events.new_surface,
	 		&server->new_xdg_surface);

    // if (server->xwayland)
    // {
    //     server->xwayland_ready.notify = server_xwayland_ready;
    //     wl_signal_add(&server->xwayland->events.ready, &server->xwayland_ready);


    //     server->new_xwayland_surface.notify = server_new_xwayland_surface;
    //     wl_signal_add(&server->xwayland->events.new_surface, &server->new_xwayland_surface);

    //     setenv("DISPLAY", server->xwayland->display_name, true);

    // }
    return true;
}
