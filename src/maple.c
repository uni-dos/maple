#include "maple_server.h"

int main(int argc, char** argv)
{
    struct maple_server server;

    server_init(&server);
    server_run(&server);

    wl_display_run(server.wl_display);

    server_destroy(&server);

    return 0;
}


