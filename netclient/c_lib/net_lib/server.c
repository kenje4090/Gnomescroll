
#ifdef DC_SERVER

#include "server.h"

#include <c_lib/agent/agent_include.h>

#include <c_lib/state/packet_init.cpp>
#include <c_lib/state/server_state.cpp>

void _NetServerInit() {
    init_message_handler();
    update_current_netpeer_time();
    init_net_lib();
    init_message_handler();
    PacketInit::RegisterMessages();
    ServerState::InitServerState();

    unsigned short port = 9999;
    init_server(port);

}

long t1 = 0;
long t2 = 0;

void _NetServerTick() {
    if(0) {
        t2 = t1;
        t1 = _get_time();
        printf("delta: %i\n",(int)(t1 - t2));
    }
        update_current_netpeer_time();

        //ServerState::ServerTick();

        process_packets(); //should poll for packets very often; event triggered packet dump

        check_pool_for_dropped_packets(); //get dropped packets before broadcasting, so retransmission can occur
        //broad_cast_packet2();
        flush_packets();

        //decrement_ttl();
        poll_connection_timeout();


        ServerState::ServerTick();  //tick after process_packets
}
#endif
