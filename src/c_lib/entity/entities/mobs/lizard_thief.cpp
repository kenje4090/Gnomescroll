#include "lizard_thief.hpp"

#include <entity/entity/_interface.hpp>
#include <entity/entity/entity.hpp>
#include <entity/constants.hpp>
#include <entity/components/health.hpp>
#include <entity/components/dimension.hpp>
#if DC_SERVER
# include <entity/entities/mobs/state_machines.hpp>
#endif

namespace Entities
{

static void lizard_thief_state_router(class Entity*, EntityState state);

void load_mob_lizard_thief_data()
{
    const EntityType type = ENTITY_MONSTER_LIZARD_THIEF;

    entity_data->begin_attaching_to(type);

    ADD_COMPONENT(PositionMomentum);

    auto dims = ADD_COMPONENT(Dimension);
    dims->height = 1.0f;

    auto mob = ADD_COMPONENT(SpriteMob);
    mob->mob.set_type("lizard_thief");

    #if DC_CLIENT
    ADD_COMPONENT(HitPoints);
    #endif
    #if DC_SERVER   // health will be set by packet initializer in client, so dont initialize it here
    auto health = ADD_COMPONENT(HitPoints);
    health->health = 80;
    health->health_max = 80;
    #endif

    auto dest = ADD_COMPONENT(DestinationTargeting);
    dest->sight_range = 15.0f;
    dest->destination_choice_x = 20;
    dest->destination_choice_y = 20;
    dest->speed = 0.125f;
    dest->max_z_diff = 5;

    auto agent = ADD_COMPONENT(AgentTargeting);
    agent->sight_range = 15.0f;
    agent->speed = 0.24f;
    agent->max_z_diff = 5;
    agent->max_lock_ticks = ONE_SECOND * 10;
    agent->proximity_radius = 2.1f;
    agent->attack_rate = (3 * ONE_SECOND) / 2;
    agent->attack_damage = 2;

    auto target = ADD_COMPONENT(WeaponTargeting);
    target->target_acquisition_failure_rate = 0.85f;
    target->fire_rate_limit = ONE_SECOND;
    target->uses_bias = false;
    target->accuracy_bias = 1.0f;
    target->sight_range = 64.0f;
    target->attack_at_random = false;
    // we dont have ID yet, need to set that in the ready() call
    target->attacker_properties.type = type;
    target->attacker_properties.block_damage = 8;
    target->attacker_properties.agent_damage_min = 2;
    target->attacker_properties.agent_damage_max = 3;
    target->attacker_properties.agent_protection_duration = ONE_SECOND * 5;
    target->attacker_properties.terrain_modification_action = TMA_TURRET;

    auto waiting = ADD_COMPONENT(Waiting);
    waiting->wait_time = ONE_SECOND * 3;

    #if DC_SERVER
    auto limiter = ADD_COMPONENT(RateLimit);
    limiter->limit = MOB_BROADCAST_RATE;

    auto item_drop = ADD_COMPONENT(ItemDrop);
    item_drop->drop->set_max_drop_types(2);
    item_drop->drop->set_max_drop_amounts("synthesizer_coin", 3);
    item_drop->drop->add_drop("synthesizer_coin", 1, 0.3f);
    item_drop->drop->add_drop("synthesizer_coin", 2, 0.1f);
    item_drop->drop->add_drop("synthesizer_coin", 3, 0.05f);
    item_drop->drop->set_max_drop_amounts("plasma_grenade", 10);
    item_drop->drop->add_drop_range("plasma_grenade", 1, 10, 0.8f);

    auto state = ADD_COMPONENT(StateMachine);
    state->state = STATE_WAITING;
    state->router = &lizard_thief_state_router;

    auto knockback = ADD_COMPONENT(Knockback);
    knockback->weight = 1.0f;
    #endif

    #if DC_CLIENT
    auto anim = ADD_COMPONENT(Animation);
    anim->color = Color(21, 150, 21);
    anim->count = 40;
    anim->count_max = 60;
    anim->size = 0.1;
    anim->force = 1.0f;
    #endif
}

#if DC_SERVER
static void lizard_thief_state_router(class Entity* entity, EntityState state)
{
    auto machine = GET_COMPONENT_INTERFACE(StateMachine, entity);

    switch (state)
    {
        case STATE_CHASE_AGENT:
            if (machine->state == STATE_WAITING)
                waiting_to_chase_agent(entity);
            else if (machine->state == STATE_IN_TRANSIT)
                in_transit_to_chase_agent(entity);
            break;

        case STATE_IN_TRANSIT:
            if (machine->state == STATE_WAITING)
                waiting_to_in_transit(entity);
            else if (machine->state == STATE_CHASE_AGENT)
                chase_agent_to_in_transit(entity);
            break;

        case STATE_WAITING:
            if (machine->state == STATE_CHASE_AGENT)
                chase_agent_to_waiting(entity);
            else if (machine->state == STATE_IN_TRANSIT)
                in_transit_to_waiting(entity);
            break;

        case STATE_NONE:
            GS_ASSERT(false);
            break;
    }
}
#endif

void tick_mob_lizard_thief(Entity* entity)
{
    #if DC_SERVER
    auto machine = GET_COMPONENT_INTERFACE(StateMachine, entity);
    auto weapon = GET_COMPONENT(WeaponTargeting, entity);

    switch (machine->state)
    {
        case STATE_WAITING:
            waiting(entity);
            break;

        case STATE_IN_TRANSIT:
            in_transit(entity);
            break;

        case STATE_CHASE_AGENT:
            if (!weapon->locked_on_target)
                chase_agent(entity);
            break;

        case STATE_NONE:
            GS_ASSERT(false);
            break;
    }

    if (machine->state != STATE_CHASE_AGENT)
    {   // aggro nearby agent
        auto physics = GET_COMPONENT_INTERFACE(Physics, entity);
        Vec3 position = physics->get_position();

        auto target = GET_COMPONENT(AgentTargeting, entity);
        target->lock_target(position);

        if (target->target_type == ENTITY_AGENT)
            machine->router(entity, STATE_CHASE_AGENT);
    }

    #endif
}

} // Entities
