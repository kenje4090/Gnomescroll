#include "player_agent_action.hpp"

#if DC_SERVER
dont_include_this_file_in_server
#endif

#include <physics/ray_trace/hitscan.hpp>
#include <agent/net_agent.hpp>
#include <sound/sound.hpp>
#include <particle/_include.hpp>
#include <particle/grenade.hpp>
#include <animations/_interface.hpp>
#include <animations/block_damage.hpp>
#include <hud/_interface.hpp>
#include <hud/cube_selector.hpp>
#include <t_map/net/t_CtoS.hpp>
#include <item/_interface.hpp>
#include <item/container/_interface.hpp>
#include <item/container/net/CtoS.hpp>
#include <item/properties.hpp>
#include <item/config/item_attribute.hpp>

namespace Agents
{

void PlayerAgent_action::hitscan_laser(ItemType weapon_type)
{
    class Agent* you = p->you();
    if (you == NULL) return;
    if (you->status.dead) return;

    Vec3 pos = agent_camera->get_position();
    Vec3 look = agent_camera->forward_vector();

    class Voxels::VoxelHitscanTarget target;
    float vox_distance;
    struct Vec3 collision_point;
    int block_pos[3];
    int side[3];
    CubeType tile;
    float block_distance;
    HitscanTargetTypes target_type = Hitscan::hitscan_against_world(
        pos, look, this->p->agent_id, OBJECT_AGENT,
        &target, &vox_distance, collision_point.f,
        block_pos, side, &tile, &block_distance);

    // for hitscan animation:
    // readjust the vector so that the translated position points to target
    // get the right vector for translating the hitscan laser anim
    Vec3 up = vec3_init(0,0,1);
    Vec3 right = vec3_cross(look, up);
    right = vec3_normalize(right);

    // magic offset numbers found by rotating the laser to the right spot
    // fixed in the bottom right corner
    const float dxy = 0.14f;
    const float dz = -0.13f;

    // animation origin
    struct Vec3 origin;
    origin.x = pos.x + dxy * right.x;
    origin.y = pos.y + dxy * right.y;
    origin.z = pos.z + dz;
    origin = translate_position(origin);

    // send packet
    hitscan_block_CtoS block_msg;
    hitscan_none_CtoS none_msg;
    hitscan_object_CtoS obj_msg;

    Agent* agent;

    int x,y,z;
    CubeType cube_type = NULL_CUBE;
    int weapon_dmg;

    switch (target_type)
    {
        case HITSCAN_TARGET_VOXEL:
            obj_msg.id = target.entity_id;
            obj_msg.type = target.entity_type;
            obj_msg.part = target.part_id;
            obj_msg.vx = target.voxel[0];
            obj_msg.vy = target.voxel[1];
            obj_msg.vz = target.voxel[2];
            obj_msg.send();

            // subtract the collision point from the origin to get the new vector for animation
            look = vec3_sub(quadrant_translate_position(origin, collision_point), origin);
            look = vec3_normalize(look);

            if (target.entity_type == OBJECT_AGENT)
            {
                Animations::blood_spray(collision_point, look);
                //Sound::play_3d_sound("laser_hit_agent", vec3_init
                    //collision_point[0], collision_point[1], collision_point[2],
                    //0,0,0
                //);
                agent = Agents::get_agent((AgentID)target.entity_id);
                if (agent==NULL) break;
            }
            break;

        case HITSCAN_TARGET_BLOCK:
            x = block_pos[0];
            y = block_pos[1];
            z = block_pos[2];

            // update predicted dmg
            // TODO -- all of this should be in the toolbelt callback probably
            // get block dmg for selected weapon
            // if block pos matched last requested block pos
            // add it to hud draw settings predicted
            // else, set it to hud draw settings predicted
            cube_type = t_map::get(x,y,z);
            GS_ASSERT(t_map::isValidCube(cube_type));
            weapon_dmg = Item::get_item_block_damage(weapon_type, cube_type);
            if (t_map::is_last_requested_block(x,y,z))
            {
                Animations::predicted_block_damage += weapon_dmg;
                Animations::damaging_block = true;
            }
            else
            {
                Animations::predicted_block_damage = weapon_dmg;
                Animations::damaging_block = true;
            }
            // make & record dmg request
            t_map::request_block_damage(x,y,z);
            block_msg.x = x;
            block_msg.y = y;
            block_msg.z = z;
            block_msg.send();

            // multiply look vector by distance to collision
            look = vec3_scalar_mult(look, block_distance);
            // add agent position, now we have collision point
            look = vec3_add(look, pos);
            // copy this to collision_point, for block damage animation
            look = translate_position(look);
            collision_point = quadrant_translate_position(current_camera_position, look);
            // subtract translated animation origin from collision point (vec) to get new vector
            look = vec3_sub(look, origin);
            look = vec3_normalize(look);
            Animations::block_damage(collision_point, look, tile, side);
            Animations::terrain_sparks(collision_point);
            //Sound::play_3d_sound("laser_hit_block", collision_point);
            break;

        case HITSCAN_TARGET_NONE:
            // for no target, leave translated animation origin
            none_msg.send();    // server will know to forward a fire weapon packet
            break;
        default:
            break;
    }

    Sound::play_2d_sound("fire_laser");
    // play laser anim (client viewport)
    const float hitscan_speed = 200.0f;
    look = vec3_scalar_mult(look, hitscan_speed);
    Animations::create_hitscan_effect(origin, look);
}

void PlayerAgent_action::update_mining_laser()
{
    class Agent* you = p->you();
    if (you == NULL) return;
    if (you->status.dead) return;
    if (!you->event.mining_laser_emitter.on) return;
    if (agent_camera == NULL) return;

    Vec3 origin = this->p->get_weapon_fire_animation_origin();
    GS_ASSERT(is_boxed_position(origin));

    struct Vec3 focal_point = vec3_add(agent_camera->get_position(), vec3_scalar_mult(agent_camera->forward_vector(), 50.0f));
    struct Vec3 direction = vec3_normalize(vec3_sub(focal_point, origin));

    you->event.mining_laser_emitter.h_mult = 0.75f;    // sprite scale offset
    you->event.mining_laser_emitter.length_position = agent_camera->get_position();
    you->event.mining_laser_emitter.length_direction = agent_camera->forward_vector();
    you->event.mining_laser_emitter.set_state(origin, direction);
    you->event.mining_laser_emitter.tick();
    you->event.mining_laser_emitter.prep_draw();
}

void PlayerAgent_action::begin_mining_laser()
{
    class Agent* you = p->you();
    if (you == NULL) return;

    ItemType laser_type = Toolbelt::get_agent_selected_item_type(you->id);
    GS_ASSERT(isValid(laser_type));
    GS_ASSERT(Item::get_item_group_for_type(laser_type) == IG_MINING_LASER);
    float range = Item::get_weapon_range(laser_type);

    you->event.mining_laser_emitter.set_base_length(range);
    you->event.mining_laser_emitter.set_laser_type(laser_type);

    you->event.mining_laser_emitter.turn_on();

    IF_ASSERT(this->mining_laser_sound_id >= 0) return;
    this->mining_laser_sound_id = Sound::play_2d_sound("mining_laser");
}

void PlayerAgent_action::end_mining_laser()
{
    class Agent* you = p->you();
    if (you == NULL) return;

    you->event.mining_laser_emitter.turn_off();

    if (this->mining_laser_sound_id < 0) return;
    Sound::stop_sound(this->mining_laser_sound_id);
    this->mining_laser_sound_id = -1;
}

void PlayerAgent_action::fire_close_range_weapon(ItemType weapon_type)
{
    class Agent* you = p->you();
    if (you == NULL) return;
    if (you->status.dead) return;

    GS_ASSERT(weapon_type != NULL_ITEM_TYPE);
    if (weapon_type == NULL_ITEM_TYPE) return;
    ItemGroup weapon_group = Item::get_item_group_for_type(weapon_type);

    float range = Item::get_weapon_range(weapon_type);

    Vec3 pos = agent_camera->get_position();
    Vec3 look = agent_camera->forward_vector();

    class Voxels::VoxelHitscanTarget target;
    float vox_distance;
    struct Vec3 collision_point;
    int block_pos[3];
    int side[3];
    CubeType tile;
    float block_distance;
    HitscanTargetTypes target_type = Hitscan::hitscan_against_world(
            pos, look, this->p->agent_id, OBJECT_AGENT,
            &target, &vox_distance, collision_point.f,
            block_pos, side, &tile, &block_distance);
    look = vec3_normalize(look);    // already normalized?
    // send packet
    hit_block_CtoS block_msg;
    melee_object_CtoS obj_msg;

    Agent* agent = NULL;
    //int voxel_blast_radius = 1;

    switch (target_type)
    {
        case HITSCAN_TARGET_VOXEL:
            if (vox_distance > range)
            {
                target_type = HITSCAN_TARGET_NONE;
                break;
            }

            obj_msg.id = target.entity_id;
            obj_msg.type = target.entity_type;
            obj_msg.part = target.part_id;
            obj_msg.vx = target.voxel[0];
            obj_msg.vy = target.voxel[1];
            obj_msg.vz = target.voxel[2];
            obj_msg.weapon_type = weapon_type;
            obj_msg.send();

            if (target.entity_type == OBJECT_AGENT)
            {
                agent = Agents::get_agent((AgentID)target.entity_id);
                if (agent==NULL)
                {
                    target_type = HITSCAN_TARGET_NONE;
                    break;
                }
                Animations::blood_spray(collision_point, look);
                //Sound::play_3d_sound("pick_hit_agent", collision_point);
                //voxel_blast_radius = 3;
            }
            //else if (target.entity_type == OBJECT_MONSTER_BOMB)
            //{
                //voxel_blast_radius = 2;
            //}
            //destroy_object_voxel(target.entity_id, target.entity_type, target.part_id, target.voxel, voxel_blast_radius);
            break;

        case HITSCAN_TARGET_BLOCK:
            if (block_distance > range)
            {
                target_type = HITSCAN_TARGET_NONE;
                break;
            }
            if (block_pos[2] >= 0 && block_pos[2] < map_dim.z)
            {
                int x = block_pos[0];
                int y = block_pos[1];
                int z = block_pos[2];

                // update predicted dmg
                // TODO -- all of this should be in the toolbelt callback probably
                // get block dmg for selected weapon
                // if block pos matched last requested block pos
                // add it to hud draw settings predicted
                // else, set it to hud draw settings predicted
                CubeType cube_type = t_map::get(x,y,z);
                GS_ASSERT(cube_type != EMPTY_CUBE);
                int weapon_dmg = Item::get_item_block_damage(weapon_type, cube_type);
                if (t_map::is_last_requested_block(x,y,z))
                {
                    Animations::predicted_block_damage += weapon_dmg;
                    Animations::damaging_block = true;
                }
                else
                {
                    Animations::predicted_block_damage = weapon_dmg;
                    Animations::damaging_block = true;
                }
                // make & record dmg request
                t_map::request_block_damage(x,y,z);
                block_msg.x = x;
                block_msg.y = y;
                block_msg.z = z;
                block_msg.weapon_type = weapon_type;
                block_msg.send();
            }

            // FOR SOME REASON COLLISION_POINT IS WORTHLESS AND WE HAVE TO CALCULATE IT HERE.

            // multiply look vector by distance to collision
            look = vec3_scalar_mult(look, block_distance);
            // add agent position, now we have collision point
            look = vec3_add(look, pos);
            // copy this to collision_point, for block damage animation
            look = translate_position(look);
            collision_point = quadrant_translate_position(current_camera_position, look);

            // subtract translated animation origin from collision point (look) to get new vector
            look = vec3_sub(collision_point, this->p->get_weapon_fire_animation_origin());
            look = vec3_normalize(look);
            Animations::block_damage(collision_point, look, tile, side);
            //Sound::play_3d_sound("pick_hit_block", collision_point);
            if (weapon_group != IG_MINING_LASER)
                Sound::play_3d_sound("block_took_damage", collision_point);
            break;

        case HITSCAN_TARGET_NONE:
            break;
    }

    if (target_type == HITSCAN_TARGET_NONE)
    {
        melee_none_CtoS none_msg;
        none_msg.send();
    }

    this->target_direction = look;
}

bool PlayerAgent_action::set_block(ItemID placer_id)
{
    class Agent* you = this->p->you();
    if (you == NULL || you->status.dead) return false;

    ItemType placer_type = Item::get_item_type(placer_id);
    IF_ASSERT(!isValid(placer_type)) return false;

    GS_ASSERT(placer_id != NULL_ITEM);
    if (placer_id == NULL_ITEM) return false;

    // get nearest empty block
    const float max_dist = 4.0f;
    class RaytraceData data;
    bool collided = raytrace_terrain(agent_camera->get_position(), agent_camera->forward_vector(), max_dist, &data);
    if (!collided) return false;

    int cube_point[3];
    data.get_pre_collision_point(cube_point);
    if (cube_point[2] <= 0 || cube_point[2] >= map_dim.z) return false; // dont modify the floor

    int orientation = axis_orientation(agent_camera->get_position(), vec3_init(cube_point[0]+0.5f, cube_point[1]+0.5f, cube_point[2]+0.5f));

    Item::ItemAttribute* attr = Item::get_item_attributes(placer_type);
    GS_ASSERT(attr != NULL);
    if (attr == NULL) return false;
    CubeType val = attr->cube_type;
    if (t_map::isItemContainer(val))
    {
        ItemContainer::create_container_block_CtoS msg;
        msg.x = cube_point[0];
        msg.y = cube_point[1];
        msg.z = cube_point[2];
        msg.placer_id = placer_id;
        msg.orientation = orientation;
        msg.send();
    }
    else
    {
        agent_set_block_CtoS msg;
        msg.x = cube_point[0];
        msg.y = cube_point[1];
        msg.z = cube_point[2];
        msg.placer_id = placer_id;
        msg.send();
    }
    return true;
}

void PlayerAgent_action::admin_set_block()
{
    class Agent* you = this->p->you();
    if (you == NULL) return;

    // get nearest empty block
    const float max_dist = 4.0f;

    class RaytraceData data;
    bool collided = raytrace_terrain(agent_camera->get_position(), agent_camera->forward_vector(), max_dist, &data);
    if (!collided) return;

    int b[3];
    data.get_pre_collision_point(b);

    if (b[2] < 0 || b[2] >= map_dim.z) return;

    int orientation = axis_orientation(agent_camera->get_position(), vec3_init(b[0]+0.5f, b[1]+0.5f, b[2]+0.5f));
    GS_ASSERT(orientation >= 0 && orientation <= 3);
    if (orientation < 0 || orientation > 3) orientation = 0;

    // get block value from somewhere
    CubeType val = HudCubeSelector::cube_selector.get_active_id();
    if (!t_map::isValidCube(val)) return;

    if (t_map::isItemContainer(val))
    {
        ItemContainer::admin_create_container_block_CtoS msg;
        msg.x = b[0];
        msg.y = b[1];
        msg.z = b[2];
        msg.val = val;
        msg.orientation = orientation;
        msg.send();
    }
    else
    {
        admin_set_block_CtoS msg;
        msg.x = b[0];
        msg.y = b[1];
        msg.z = b[2];
        msg.val = val;
        msg.send();
    }
}

void PlayerAgent_action::throw_grenade()
{
    class Agent* you = p->you();
    if (you == NULL) return;
    if (you->status.dead) return;

    // message to server
    Vec3 pos = you->get_camera_position();
    ThrowGrenade_CtoS msg;
    msg.x = pos.x;
    msg.y = pos.y;
    msg.z = pos.z;
    Vec3 f = you->forward_vector();    // use network state
    msg.vx = f.x;
    msg.vy = f.y;
    msg.vz = f.z;
    msg.send();

    // local play (copied from ThrowGrenade_CtoS)
    static const float PLAYER_ARM_FORCE = 15.0f; // make agent property
    //create grenade
    f = vec3_scalar_mult(f, PLAYER_ARM_FORCE);
    Particle::Grenade* g = Particle::grenade_list->create();
    GS_ASSERT(g != NULL);
    if (g == NULL) return;
    g->set_state(pos.x, pos.y, pos.z, f.x, f.y, f.z);
    g->owner = this->p->agent_id;
}

void PlayerAgent_action::place_spawner()
{
    class Agent* you = p->you();
    if (you == NULL) return;
    if (you->status.dead) return;

    const float max_dist = 4.0f;

    class RaytraceData data;
    bool collided = raytrace_terrain(agent_camera->get_position(), agent_camera->forward_vector(), max_dist, &data);
    if (!collided) return;

    int block[3];
    data.get_pre_collision_point(block);

    place_spawner_CtoS msg;
    msg.x = block[0];
    msg.y = block[1];
    msg.z = block[2];
    msg.send();
}

void PlayerAgent_action::place_turret()
{
    class Agent* you = p->you();
    if (you == NULL) return;
    if (you->status.dead) return;

    const float max_dist = 4.0f;

    class RaytraceData data;
    bool collided = raytrace_terrain(agent_camera->get_position(), agent_camera->forward_vector(), max_dist, &data);
    if (!collided) return;

    int block[3];
    data.get_pre_collision_point(block);

    place_turret_CtoS msg;
    msg.x = block[0];
    msg.y = block[1];
    msg.z = block[2];
    msg.send();
}

Vec3 PlayerAgent_action::get_aiming_point()
{
    class Agent* you = p->you();
    if (you == NULL) return vec3_init(0,0,0);
    if (you->status.dead) return vec3_init(0,0,0);

    Vec3 pos = agent_camera->get_position();
    Vec3 look = agent_camera->forward_vector();

    class Voxels::VoxelHitscanTarget target;
    float vox_distance;
    float collision_point[3];
    int block_pos[3];
    int side[3];
    CubeType tile;
    float block_distance;

    HitscanTargetTypes target_type =
        Hitscan::hitscan_against_world(
            pos, look, this->p->agent_id, OBJECT_AGENT,
            &target, &vox_distance, collision_point,
            block_pos, side, &tile, &block_distance
        );
    if (target_type == HITSCAN_TARGET_VOXEL) return vec3_init(collision_point[0], collision_point[1], collision_point[2]);
    else if (target_type == HITSCAN_TARGET_BLOCK) return vec3_add(pos, vec3_scalar_mult(look, block_distance));
    else return vec3_init(0,0,0);
}

PlayerAgent_action::PlayerAgent_action(PlayerAgent* player_agent) :
    p(player_agent), mining_laser_sound_id(-1)
{}

}   // Agents
