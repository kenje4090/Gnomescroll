#include "policy.hpp"

#include <c_lib/objects/common/net/packets.hpp>

namespace Objects
{
void broadcastDeath(ObjectState* state)
{
    #if DC_SERVER
    object_destroy_StoC msg;
    msg.id = state->id;
    msg.type = state->type;
    msg.broadcast();
    #endif
}
} //Objects
