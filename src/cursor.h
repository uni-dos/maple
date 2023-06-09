#ifndef MAPLE_CURSOR_H
#define MAPLE_CURSOR_H

#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_xcursor_manager.h>

extern struct maple_server server;

bool setup_cursor(struct maple_server *server);
#endif
