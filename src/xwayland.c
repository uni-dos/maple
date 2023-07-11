#include "view.h"
#include "xwayland.h"

void server_xwayland_ready(struct wl_listener *listener, void * data)
{
    (void) data;
    struct maple_server *server = wl_container_of(listener, server, xwayland_ready);

    wlr_xwayland_set_seat(server->xwayland, server->seat);
}

void server_new_xwayland_surface(struct wl_listener *listener, void * data)
{
    struct maple_server *server = wl_container_of(listener, server, new_xwayland_surface);

    struct wlr_xwayland_surface *xwayland_surface = data;

    struct maple_view *view = calloc(1, sizeof(struct maple_view));

    view->server = server;
    view->view_type = VIEW_X11;
    view->xwayland_surface = xwayland_surface;

 //    view->map.notify = xdg_toplevel_map;
	// wl_signal_add(&xwayland_surface->surface->events.map, &view->map);

	// view->unmap.notify = xdg_toplevel_unmap;
	// wl_signal_add(&xwayland_surface->surface->events.unmap, &view->unmap);

	// view->destroy.notify = xdg_toplevel_destroy;
	// wl_signal_add(&xwayland_surface->events.destroy, &view->destroy);

    //TODO
	// view->activate.notify = activate;
	// wl_signal_add(&xwayland_surface->events.request_activate,
	//     &view->activate);

	// view->configure.notify = configure;
	// wl_signal_add(&xwayland_surface->events.request_configure,
	//     &view->configure);


}
