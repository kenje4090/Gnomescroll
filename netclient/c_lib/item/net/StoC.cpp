#include "StoC.hpp"

#if DC_CLIENT

#include <item/_state.hpp>
#include <t_hud/_interface.hpp>

namespace Item
{

// Item

inline void item_create_StoC::handle()
{
}

// Containers

inline void create_item_container_StoC::handle()
{
    ItemContainer* ic = item_container_list->create(container_id);
    init_container(ic, (ItemContainerType)container_type);
}

inline void delete_item_container_StoC::handle()
{
    item_container_list->destroy(container_id);
}

inline void assign_item_container_StoC::handle()
{
    ItemContainer* ic = item_container_list->get(container_id);
    ASSERT_NOT_NULL(ic);
    ItemContainerType type = (ItemContainerType) container_type;
    switch (type)
    {
        case AGENT_INVENTORY:
            player_inventory_id = container_id;
            player_inventory = ic;
            break;
        case AGENT_TOOLBELT:
            player_toolbelt_id = container_id;
            player_toolbelt = ic;
            break;
        case AGENT_NANITE:
            player_nanite_id = container_id;
            player_nanite = ic;
            break;
        default:
            assert(false);
            break;
    }

}

// Action

inline void container_action_failed_StoC::handle()
{
    // copy network state to render state
}

} // Item
#endif

#if DC_SERVER

namespace Item
{

// dummies

inline void item_create_StoC::handle() {}
    
inline void create_item_container_StoC::handle() {}
inline void delete_item_container_StoC::handle() {}
inline void assign_item_container_StoC::handle() {}

inline void container_action_failed_StoC::handle() {}

} // Item

#endif
