#include <wlr/util/log.h>
#include "server.h"
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
    server_run(&server);

    server_destroy(&server);

    return 0;
}


