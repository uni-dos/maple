#ifndef MAPLE_OUTPUT
#define MAPLE_OUTPUT
#include "server.h"

struct maple_output {

    struct wlr_output *wlr_output;
    struct maple_server *server;

};

bool set_up_output(struct maple_server *server);
#endif
