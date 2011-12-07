#include "export.hpp"

//typedef void (*PY_MESSAGE_CALLBACK)(char* buff, int n, int client_id);
PY_MESSAGE_CALLBACK PY_MESSAGE_CALLBACK_GLOBAL = NULL;

void set_python_net_callback_function(PY_MESSAGE_CALLBACK pt)
{  
    PY_MESSAGE_CALLBACK_GLOBAL = pt;
}



void send_python_net_message(char* message, int length, int client_id)
{
    #ifdef DC_CLIENT
        static int t_count = 0;
        printf("py_out: packet %i \n", t_count);
        NetClient::NPserver.write_python_packet(message, length);
        t_count++;
    #endif

    #ifdef DC_SERVER
        if(NetServer::pool.connection[client_id] == NULL)
        {
            printf("send_python_net_message: client_id % is null\n", client_id);
            return;
        }
        NetServer::pool.connection[client_id]->write_python_packet(message, length);
    #endif    

}