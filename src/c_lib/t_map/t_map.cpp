#include "t_map.hpp"

#include <t_map/_interface.hpp>
#include <t_map/t_properties.hpp>
#include <t_map/t_map_class.hpp>

#if DC_CLIENT
# include <t_map/glsl/cache.hpp>
# include <t_map/glsl/texture.hpp>
# include <t_map/glsl/shader.hpp>
# include <t_map/net/t_StoC.hpp>
#endif

#if DC_SERVER
# include <t_map/server/manager.hpp>
# include <t_map/server/subscription_list.hpp>
# include <t_map/server/env_process.hpp>
# include <common/random.hpp>
# include <particle/_interface.hpp>
# include <item/_interface.hpp>
# include <t_map/config/drop_dat.hpp>
# include <entity/object/main.hpp>
# include <t_gen/explosives.hpp>
#endif

namespace t_map
{

class Terrain_map* main_map = NULL;

CubeID get(int x, int y, int z)
{
    if((z & TERRAIN_MAP_HEIGHT_BIT_MASK) != 0) return EMPTY_CUBE;
    x &= TERRAIN_MAP_WIDTH_BIT_MASK2;
    y &= TERRAIN_MAP_WIDTH_BIT_MASK2;
    class MAP_CHUNK* c = main_map->chunk[ MAP_CHUNK_XDIM*(y >> 4) + (x >> 4) ];
    if (c == NULL) return EMPTY_CUBE;
    return (CubeID)c->e[ (z<<8)+((y&15)<<4)+(x&15) ].block;

}

CubeID set(int x, int y, int z, CubeID cube_id)
{
    #if DC_SERVER
    CubeID existing_cube_id = t_map::get(x,y,z);
    // dont overwrite existing cubes with new ones
    //IF_ASSERT(cube_id != EMPTY_CUBE && existing_cube_id != EMPTY_CUBE) return existing_cube_id;

    // TODO -- make a single handler for this item container check,
    // and the explosives checks (any special destruction required blocks)
    // and call it here (along with the apply_damage location)
    if (isItemContainer(existing_cube_id))
        destroy_item_container_block(x,y,z);
    #endif
    main_map->set_block(x,y,z, cube_id);
    return cube_id;
}

OPTIMIZED
void set_fast(int x, int y, int z, CubeID cube_id)
{
    main_map->set_block(x,y,z, cube_id);
}

struct MAP_ELEMENT get_element(int x, int y, int z)
{
    if((z & TERRAIN_MAP_HEIGHT_BIT_MASK) != 0) return NULL_MAP_ELEMENT;
    x &= TERRAIN_MAP_WIDTH_BIT_MASK2;
    y &= TERRAIN_MAP_WIDTH_BIT_MASK2;
    class MAP_CHUNK* c = main_map->chunk[ MAP_CHUNK_XDIM*(y >> 4) + (x >> 4) ];
    if(c == NULL) return NULL_MAP_ELEMENT;
    return c->e[ (z<<8)+((y&15)<<4)+(x&15) ];
}

void set_palette(int x, int y, int z, int palette)
{
    struct MAP_ELEMENT element = main_map->get_element(x,y,z);
    element.palette = palette;
    main_map->set_element(x,y,z,element);
}

int get_palette(int x, int y, int z)
{
    return main_map->get_element(x,y,z).palette;
}


class Terrain_map* get_map()
{
    return main_map;
}

void init_t_map()
{   
    main_map = new Terrain_map(MAP_WIDTH, MAP_HEIGHT); //512 by 512 map

    #if DC_CLIENT
    init_client_compressors();
    init_t_vbo();
    init_textures();
    #endif

    #if DC_SERVER
    map_history = new Terrain_map_subscription(MAP_WIDTH, MAP_HEIGHT);
    init_env_process();
    #endif
}

#if DC_CLIENT
void init_for_draw()
{
    init_cache();
    control_node_render_init(&main_map->control_node_list);
    init_shaders();
}
#endif
    
void end_t_map()
{
    if (main_map != NULL) delete main_map;

    #if DC_CLIENT
    end_client_compressors();
    teardown_t_vbo();
    control_node_render_teardown();
    #endif

    #if DC_SERVER
    if (map_history != NULL) delete map_history;
    teardown_env_process();
    #endif
}

int get_block_damage(int x, int y, int z)
{
    if((z & TERRAIN_MAP_HEIGHT_BIT_MASK) != 0) return 0;
    x &= TERRAIN_MAP_WIDTH_BIT_MASK2;
    y &= TERRAIN_MAP_WIDTH_BIT_MASK2;

    return main_map->get_damage(x,y,z);
}

#if DC_SERVER

// apply block damage & broadcast the update to client
void apply_damage_broadcast(int x, int y, int z, int dmg, TerrainModificationAction action)
{
    IF_ASSERT(dmg <= 0) return;
    IF_ASSERT((z & TERRAIN_MAP_HEIGHT_BIT_MASK) != 0) return;
    x &= TERRAIN_MAP_WIDTH_BIT_MASK2;
    y &= TERRAIN_MAP_WIDTH_BIT_MASK2;

    CubeID cube_id = ERROR_CUBE;
    int ret = t_map::main_map->apply_damage(x,y,z, dmg, &cube_id);
    if (ret != 0) return;

    // always explode explosives with force
    if (isExplosive(cube_id)) action = TMA_PLASMAGEN;

    // block_action packet expects final value of cube, not initial value
    map_history->send_block_action(x,y,z, EMPTY_CUBE, action);

    if (cube_properties[cube_id].item_drop) 
        handle_block_drop(x,y,z, cube_id);
}

void broadcast_set_block_action(int x, int y, int z, CubeID cube_id, int action)
{
    IF_ASSERT((z & TERRAIN_MAP_HEIGHT_BIT_MASK) != 0) return;
    x &= TERRAIN_MAP_WIDTH_BIT_MASK2;
    y &= TERRAIN_MAP_WIDTH_BIT_MASK2;

    map_history->send_block_action(x,y,z, cube_id, action);
}

void broadcast_set_block(int x, int y, int z, CubeID cube_id)
{
    IF_ASSERT((z & TERRAIN_MAP_HEIGHT_BIT_MASK) != 0) return;
    x &= TERRAIN_MAP_WIDTH_BIT_MASK2;
    y &= TERRAIN_MAP_WIDTH_BIT_MASK2;

    main_map->set_block(x,y,z, cube_id);
    map_history->send_set_block(x,y,z, cube_id);
}

void broadcast_set_block_palette(int x, int y, int z, CubeID block, int palette)
{
    IF_ASSERT((z & TERRAIN_MAP_HEIGHT_BIT_MASK) != 0) return;
    x &= TERRAIN_MAP_WIDTH_BIT_MASK2;
    y &= TERRAIN_MAP_WIDTH_BIT_MASK2;

    struct MAP_ELEMENT e = {{{0}}};
    e.block = block;
    e.palette = palette;

    main_map->set_element(x,y,z,e);
    map_history->send_set_block_palette(x,y,z, block,palette);
}

void broadcast_set_palette(int x, int y, int z, int palette)
{
    IF_ASSERT((z & TERRAIN_MAP_HEIGHT_BIT_MASK) != 0) return;
    x &= TERRAIN_MAP_WIDTH_BIT_MASK2;
    y &= TERRAIN_MAP_WIDTH_BIT_MASK2;

    struct MAP_ELEMENT e = get_element(x,y,z);
    e.palette = palette;

    main_map->set_element(x,y,z,e);
    map_history->send_set_block_palette(x,y,z, (CubeID)e.block, e.palette);
}

#endif

inline int get_highest_open_block(int x, int y, int n)
{
    IF_ASSERT(n < 1) return -1;
    if (n == 1) return get_highest_open_block(x, y);

    int open = n;
    CubeID cube_id;
    int i;

    for (i=map_dim.z-1; i>=0; i--)
    {
        cube_id = get(x, y, i);
        if (!isSolid(cube_id))
            open++;
        else
        {
            if (open >= n) return i+1;
            open = 0;
        }
    }
    if (open >= n) return 0;
    return -1;
}

inline int get_highest_open_block(int x, int y) 
{
    return get_highest_solid_block(x, y) + 1;
}

inline int get_nearest_open_block(int x, int y, int z, int n)
{
    IF_ASSERT(n < 1) return z;

    int inc = 1;    // direction (+/- up/down)
    int up = z;     // up cursor
    int down = z-1; // down cursor
    while (down >= 0 && up < map_dim.z)
    {
        // choose start pt
        int start = up;
        if (inc < 0)
            start = down;
            
        // look for gap of height n
        bool found = true;
        for (int j=0; j<n; j++)
        {
            if (isSolid(x,y,start+j))
            {
                found = false;
                break;
            }
        }
        if (found) return start;
        
        // advance up/down cursors
        if (inc > 0)
            up++;
        else
            down--;
        
        // set direction
        if (up >= map_dim.z)
            inc = -1;
        else if (down < 0)
            inc = 1;
        else
            inc *= -1;  // flip
    }
    
    return -1;
}

inline int get_nearest_open_block(int x, int y, int z)
{
    return get_nearest_open_block(x,y,z,1);
}

inline int get_solid_block_below(int x, int y, int z)
{
    for (int i=z-1; i>=0; i--)
        if (get(x,y,i) != 0) return i;
    return -1;
}

inline int get_highest_solid_block(int x, int y)
{
    return get_highest_solid_block(x,y, map_dim.z);
}

inline int get_highest_solid_block(int x, int y, int z)
{
    //#if DC_CLIENT
    // HEIGHTMAP CACHE IS INVALID
    //return main_map->get_cached_height(x,y)-1;
    //#endif

    //#if DC_SERVER
    int i = z-1;
    for (; i>=0; i--)
        if (isSolid(x,y,i)) break;
    return i;
    //#endif
}

inline int get_lowest_open_block(int x, int y, int n)
{
    IF_ASSERT(n < 1) return -1;

    int i;
    CubeID cube_id;
    int open=0;
    for (i=0; i<map_dim.z; i++)
    {
        cube_id = get(x,y,i);
        if (isSolid(cube_id)) open = 0;
        else open++;
        if (open >= n) return i-open+1;
    }

    return i;
}

inline int get_lowest_solid_block(int x, int y)
{
    int i;
    for (i=0; i<map_dim.z; i++)
        if (isSolid(x,y,i)) break;
    if (i >= map_dim.z) i = -1;  // failure
    return i;
}

inline bool position_is_loaded(int x, int y)
{
    #if DC_SERVER
    return true;
    #endif

    #if DC_CLIENT
    x &= TERRAIN_MAP_WIDTH_BIT_MASK2;
    y &= TERRAIN_MAP_WIDTH_BIT_MASK2;
    int cx = x / TERRAIN_CHUNK_WIDTH;
    int cy = y / TERRAIN_CHUNK_WIDTH;
    int index = main_map->xchunk_dim*cy+cx;

    //if map chunk is null, it is not loaded
    return (main_map->chunk[index] != NULL);
    #endif
}

bool block_can_be_placed(int x, int y, int z, CubeID cube_id)
{
    IF_ASSERT((z & TERRAIN_MAP_HEIGHT_BIT_MASK) != 0) return false;
    x &= TERRAIN_MAP_WIDTH_BIT_MASK2;
    y &= TERRAIN_MAP_WIDTH_BIT_MASK2;

    bool valid_cube = isValidCube(cube_id);
    IF_ASSERT(!valid_cube) return false;
    
    if (get(x,y,z) != EMPTY_CUBE) return false;
    // check against all spawners
    if (Entities::point_occupied_by_type(OBJECT_AGENT_SPAWNER, x,y,z))
        return false;
    if (Entities::point_occupied_by_type(OBJECT_ENERGY_CORE, x,y,z))
        return false;
    return true;
}

}   // t_map
