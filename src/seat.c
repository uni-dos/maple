#include "seat.h"
#include "server.h"
#include <stdlib.h>



 static void setup_cursor(struct maple_server *server)
 {

     /* allocate memory for the maple cursor
     * because there is no abstraction that does it for us */
     server->seat = calloc(1, sizeof(struct maple_seat));

     /* Creates a cursor to track on screen */
     server->seat->cursor = wlr_cursor_create();


     char *xcursor_theme = getenv("XCURSOR_THEME");
     char *xcursor_size = getenv("XCURSOR_SIZE");

     //if XCURSOR_SIZE was not set, make it 24
     int size = xcursor_size ? strtoul(xcursor_size, NULL,  10) : 24;

     /* Creates an xcursor manager, another wlroots utility which loads up
     * Xcursor themes to source cursor images from and makes sure that cursor
     * images are available at all scale factors on the screen (necessary for
     * HiDPI support). We add a cursor theme at scale factor 1 to begin with. */
     server->seat->cursor_mngr = wlr_xcursor_manager_create(xcursor_theme, size);

     // returns a bool
     wlr_xcursor_manager_load(server->seat->cursor_mngr, 1);
}
