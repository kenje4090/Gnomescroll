#pragma once

/*
	Client
*/
#ifdef DC_CLIENT

#include <c_lib/t_item/free_item.cpp>
#include <c_lib/t_item/_interface.cpp>
#include <c_lib/t_item/item_container.cpp>
#include <c_lib/t_item/item.cpp>

#include <c_lib/t_item/sprite.cpp>

#include <c_lib/t_item/net/CtoS.cpp>
#include <c_lib/t_item/net/StoC.cpp>

#endif

/*
	Server
*/

#ifdef DC_SERVER

#include <c_lib/t_item/free_item.cpp>
#include <c_lib/t_item/_interface.cpp>
#include <c_lib/t_item/item_container.cpp>
#include <c_lib/t_item/item.cpp>

#include <c_lib/t_item/server/agent_inventory.cpp>

#include <c_lib/t_item/net/CtoS.cpp>
#include <c_lib/t_item/net/StoC.cpp>
	
#endif