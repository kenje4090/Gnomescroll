#include "agent_event.hpp"

#if DC_SERVER
dont_include_this_file_in_server
#endif

#include <input/handlers.hpp>

#include <state/client_state.hpp>
#include <sound/triggers.hpp>
#include <chat/client.hpp>
#include <hud/map.hpp>

#include <particle/_interface.hpp>
#include <particle/constants.hpp>

#include <animations/_interface.hpp>
#include <animations/animations.hpp>

#include <item/properties.hpp>

void Agent_event::name_changed(char* old_name)
{
	GS_ASSERT(this->a->status.name != NULL);
	GS_ASSERT(old_name != NULL);
	if (this->a->status.name == NULL || old_name == NULL) return;
	
	this->bb.set_text(this->a->status.name);

    if (a->is_you())
    {
        const char fmt[] = "You are identified as %s";
        char* msg = (char*)calloc(strlen(fmt) + strlen(this->a->status.name) - 2 + 1, sizeof(char));
        sprintf(msg, fmt, this->a->status.name);
        chat_client->send_system_message(msg);
        free(msg);
    }
    else
    {
        if (strcmp(AGENT_UNDEFINED_NAME, old_name))
        {
            const char fmt[] = "%s is now known as %s";
            char* msg = (char*)calloc(strlen(fmt) + strlen(old_name) + strlen(this->a->status.name) - 4 + 1, sizeof(char));
            sprintf(msg, fmt, old_name, this->a->status.name);
            chat_client->send_system_message(msg);
            free(msg);
        }
        else
        {
            const char fmt[] = "%s has joined the game";
            char* msg = (char*)calloc(strlen(fmt) + strlen(this->a->status.name) - 2 + 1, sizeof(char));
            sprintf(msg, fmt, this->a->status.name);
            chat_client->send_system_message(msg);
            free(msg);
            #if DC_CLIENT && PRODUCTION_DEV
            Sound::player_joined_server();  // move this to a better place, along with the other dis/connect events
            #endif
        }
    }

    this->a->status.identified = true;
}

void Agent_event::hide_name()
{
    this->bb.set_draw(false);
}

void Agent_event::display_name()
{
    if (this->a->status.dead) return;

    const float z_margin = 0.4;
    Vec3 p = this->a->get_position();
    this->bb.set_state(p.x, p.y, p.z + a->current_height() + z_margin, 0.0f, 0.0f, 0.0f);
    this->bb.set_draw(true);
}

// side effects of taking damage. dont modify health/death here
void Agent_event::took_damage(int dmg)
{
    GS_ASSERT(dmg > 0);
    if (dmg <= 0) return;
    Particle::BillboardText* b = Particle::billboard_text_list->create();
    GS_ASSERT(b != NULL);
    if (b==NULL) return;
    b->init();

    Vec3 p = this->a->get_position();
    b->set_state(
        p.x + (randf()*(a->box.box_r*2) - a->box.box_r),
        p.y + (randf()*(a->box.box_r*2) - a->box.box_r),
        p.z + a->current_height(),
        0.0f,0.0f, BB_PARTICLE_DMG_VELOCITY_Z
    );
    b->set_color(BB_PARTICLE_DMG_COLOR);   // red
    char txt[10+1];
    sprintf(txt, "%d", dmg);
    b->set_text(txt);
    b->set_scale(1.0f);
    b->set_ttl(245);

    if (a->is_you())
        Sound::agent_took_damage();
    // TODO: attenuated damage sound
    //else
        //Sound::agent_took_damage(p.x, p.y, p.z, 0,0,0);
}

void Agent_event::healed(int amount)
{
    GS_ASSERT(amount >= 0);
    if (a->is_you())
    {
        Sound::restore_health();
        chat_client->send_system_message((char*) "You healed.");
    }
    else
    {
        Vec3 p = this->a->get_position();
        Sound::restore_health(p.x, p.y, p.z, 0,0,0);
    }

    // show billboard text particle
    Particle::BillboardText* b = Particle::billboard_text_list->create();
    GS_ASSERT(b != NULL);
    if (b==NULL) return;
    b->init();

    Vec3 p = this->a->get_position();
    b->set_state(
        p.x + (randf()*(a->box.box_r*2) - a->box.box_r),
        p.y + (randf()*(a->box.box_r*2) - a->box.box_r),
        p.z + a->current_height(),
        0.0f,0.0f, BB_PARTICLE_HEAL_VELOCITY_Z
    );
    b->set_color(BB_PARTICLE_HEAL_COLOR);   // red
    char txt[10+1];
    sprintf(txt, "%d", amount);
    b->set_text(txt);
    b->set_scale(1.0f);
    b->set_ttl(245);
}

void Agent_event::died()
{
    if (!this->a->status.dead)
    {
        this->a->status.dead = true;
        if (a->is_you())
        {
            close_all_containers();
            Sound::died();
        }
        else
        {
            Vec3 p = this->a->get_position();
            Sound::died(p.x, p.y, p.z, 0,0,0);
        }
        this->a->vox->set_vox_dat(&VoxDats::agent_dead);
        this->a->vox->reset_skeleton();
    }
    Toolbelt::agent_died(this->a->id);
}

void Agent_event::born()
{
    if (this->a->status.dead)
    {
        if (a->is_you())
            Sound::respawned();
        //else
            //Sound::respawned(a->s.x, a->s.y, a->s.z, 0,0,0);
        this->a->status.dead = false;

        // reset skeleton
        VoxDat* vd = (this->a->crouched()) ? &VoxDats::agent_crouched : &VoxDats::agent;
        this->a->vox->set_vox_dat(vd);
        this->a->vox->reset_skeleton();
    }
}

void Agent_event::life_changing(bool dead)
{
    if (dead) died();
    else born();
}

void Agent_event::set_spawner(int pt)
{
    this->a->status.spawner = pt;
    //printf("Agent_event:: set spawner to %d\n", pt);
}

void Agent_event::crouched()
{
    this->model_was_changed = true;
    this->a->vox->set_vox_dat(&VoxDats::agent_crouched);
    this->a->vox->reset_skeleton();
}

void Agent_event::uncrouched()
{
    this->model_was_changed = true;
    this->a->vox->set_vox_dat(&VoxDats::agent);
    this->a->vox->reset_skeleton();
}

bool Agent_event::model_changed()
{
    bool changed = this->model_was_changed;
    this->model_was_changed = false;
    return changed;
}

void Agent_event::set_agent_vox_status(AgentVoxStatus status)
{
    if (this->vox_status != status) this->model_was_changed = true;
    this->vox_status = status;
}

void Agent_event::reload_weapon(int type)
{
    Vec3 p = this->a->get_position();
    Sound::reload(p.x, p.y, p.z, 0,0,0);
    // play reload animation/sound for the weapon
}

void Agent_event::tick_mining_laser()
{
    int weapon_type = Item::get_item_type((char*)"mining_laser");
    float range = Item::get_weapon_range(weapon_type);

    Animations::mining_laser_beam(this->a->arm_center(), this->a->forward_vector(), range);
}

void Agent_event::fired_mining_laser()
{
    //Sound::fire_mining_laser();
}

void Agent_event::fired_weapon_at_object(int id, int type, int part)
{
    AgentState s = this->a->get_state();
    s.z = this->a->camera_z();

    Sound::fire_laser(s.x, s.y, s.z, s.vx, s.vy, s.vz);

    Vec3 f = this->a->forward_vector();

    if (type == OBJECT_AGENT)
    {
        Agent_state* agent = ClientState::agent_list->get(id);
        if (agent != NULL && agent->vox != NULL)
        {
            Voxel_volume* vv = agent->vox->get_part(part);
            if (vv != NULL)
            {
                float c[3];
                vv->get_center(c);
                Animations::blood_spray(c[0], c[1], c[2], f.x, f.y, f.z);
                Sound::pick_hit_agent(  // TODO: switch (weapon) {}
                    c[0], c[1], c[2],
                    0,0,0
                );
            }
        }
    }

    if (this->a->vox == NULL) return;
    // play laser anim out of arm
    const float hitscan_speed = 200.0f;
    Vec3 arm_center = this->a->arm_center();

	f = vec3_scalar_mult(f, hitscan_speed);
    
    Animations::create_hitscan_effect(
        arm_center.x, arm_center.y, arm_center.z,
        f.x, f.y, f.z
    );

}

void Agent_event::fired_weapon_at_block(float x, float y, float z, int cube, int side)
{
    AgentState s = this->a->get_state();
    s.z = this->a->camera_z();

    Sound::fire_laser(s.x, s.y, s.z, s.vx, s.vy, s.vz);

    if (this->a->vox == NULL) return;

    // animate laser to target

    // play laser anim out of arm
    const float hitscan_speed = 200.0f;
    //Vec3 arm_center = this->a->vox->get_part(AGENT_PART_RARM)->world_matrix.c;
    Vec3 arm_center = this->a->vox->get_node(5)->c;
    
    // vector from arm center to collision point
    Vec3 p = vec3_init(x,y,z);
    p = quadrant_translate_position(this->a->get_position(), p);
    Vec3 f = vec3_sub(p, arm_center);
    normalize_vector(&f);

    f = vec3_scalar_mult(f, hitscan_speed);
    Animations::create_hitscan_effect(
        arm_center.x, arm_center.y, arm_center.z,
        f.x, f.y, f.z
    );

    // play block surface crumble
    Animations::block_damage(x,y,z, f.x, f.y, f.z, cube, side);
    Animations::terrain_sparks(x,y,z);
    Sound::laser_hit_block(x,y,z, 0,0,0);
}

void Agent_event::fired_weapon_at_nothing()
{
    AgentState s = this->a->get_state();
    s.z = this->a->camera_z();

    Sound::fire_laser(s.x, s.y, s.z, s.vx, s.vy, s.vz);

    if (this->a->vox == NULL) return;
    
    Vec3 f = this->a->forward_vector();
    
    // play laser anim out of arm
    const float hitscan_speed = 200.0f;
	f = vec3_scalar_mult(f, hitscan_speed);
    Vec3 arm_center = this->a->vox->get_part(AGENT_PART_RARM)->world_matrix.c;
    Animations::create_hitscan_effect(
        arm_center.x, arm_center.y, arm_center.z,
        f.x, f.y, f.z
    );
}

void Agent_event::threw_grenade()
{
    // play throw grenade animation
    // might need to sync grenades with this?
}

void Agent_event::placed_block()
{
    // player agent block placement animation
}

void Agent_event::hit_block()
{
    // play pick swing
    // play block damage animation
    //Vec3 p = this->a->get_camera_position();
    //Sound::pick_swung(p.x, p.y, p.z, 0,0,0);

    // TODO -- need collision point
    //Sound::block_took_damage(collision_point[0], collision_point[1], collision_point[2], 0,0,0);
}

void Agent_event::melee_attack_object(int id, int type, int part)
{
    // play pick swing animation
    // play blood animation
    // play swing sound
    // play object's hurt sound

    //Vec3 p = this->a->get_camera_position();
    //Sound::pick_swung(p.x,p.y,p.z,0,0,0);
    //Sound::pick_hit_agent(p.x, p.y, p.z,0,0,0);
}

void Agent_event::melee_attack_nothing()
{
    // play pick swing animation
    //Vec3 p = this->a->get_camera_position();
    //Sound::pick_swung(p.x, p.y, p.z, 0,0,0);
}

void Agent_event::fire_empty_weapon(int weapon_type)
{
    Vec3 p = this->a->get_camera_position();
    Sound::out_of_ammo(p.x, p.y, p.z, 0,0,0);
}

Agent_event::~Agent_event()
{
}

Agent_event::Agent_event(Agent_state* owner)
:
a(owner),
vox_status(AGENT_VOX_IS_STANDING),
model_was_changed(true)
{
	this->bb.init();
	this->bb.permanent = true;          // dont die
	if (this->a->status.name != NULL)
		this->bb.set_text(this->a->status.name);
	this->bb.set_color(255,10,10,255); // TODO -- use health level
	this->bb.set_scale(AGENT_HUD_NAME_SIZE);	
}
