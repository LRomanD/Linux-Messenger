#include "hdr/configurator.h"
#include "hdr/network.h"
#include "hdr/graphics.h"

#include <stdio.h>
#include <locale.h>

int main(void)
{
    int ret = 1;
    setlocale (LC_ALL, "");

    read_config();
    // init_graphics();
    puts("connecting");
    if (connect_to_main_server() == EXIT_SUCCESS)
    {
        // while(ret > 0)
        // {
        //     switch (ret = menu_wnd())
        //     {
        //         case 1:
        //             ret = join_srv_wnd();
        //             break;
        //         case 2:
        //             ret = create_srv_wnd();
        //             break;
        //         case 3:
        //             ret = prefs_wnd();
        //             break;
        //         case -1:
        //             /* code */
        //             break;
        //         default:
        //             break;
        //     }
        // }
    }
    ret = get_latency();
    printf("Latency = %d mcs\n", ret);
    puts("joining");
    ret = client_send(JOIN_COMM, NET_WAIT_FALSE, "255.255.255.255", 25519);
    printf("ret = %d\n", ret);
    puts("disconnecting");
    disconnect_from_main_server();

    // deinit_graphics();

    return 0;
}