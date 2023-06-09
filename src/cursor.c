#include "cursor.h"
#include "server.h"
#include <stdlib.h>

bool setup_cursor(struct maple_server *server) {

    /* Creates a cursor to track on screen */
    server->cursor = wlr_cursor_create();
    if (!server->cursor) {
        return false;
    }
    server->cursor_mode = CURSOR_PASSTHROUGH;

    char *xcursor_theme = getenv("XCURSOR_THEME");
    char *xcursor_size = getenv("XCURSOR_SIZE");

    //if XCURSOR_SIZE was not set, make it 24
    int size = xcursor_size ? strtoul(xcursor_size, nullptr, 10) : 24;

    /* Creates an xcursor manager, another wlroots utility which loads up
    * Xcursor themes to source cursor images from and makes sure that cursor
    * images are available at all scale factors on the screen (necessary for
    * HiDPI support). We add a cursor theme at scale factor 1 to begin with. */
    server->cursor_mngr = wlr_xcursor_manager_create(xcursor_theme, size);


    wlr_xcursor_manager_load(server->cursor_mngr, 1);

    // TODO create methods for the events total of 5
    return true;
}


