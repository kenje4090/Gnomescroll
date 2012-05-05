#include "_interface.hpp"

#include <agent/agent.hpp>

#include <item/_interface.hpp>
#include <item/particle/item_particle.hpp>
#include <item/particle/net/StoC.hpp>

namespace ItemParticle
{

class ItemParticle_list* item_particle_list = NULL;

void init()
{
    item_particle_list = new ItemParticle_list;
}

void teardown()
{
    delete item_particle_list;
}

void tick()
{
    assert(item_particle_list != NULL);
    item_particle_list->tick();
}

void destroy(int particle_id)
{
    item_particle_list->destroy(particle_id);
}

#if DC_CLIENT
void draw_init()
{
    init_item_particle();
}

void draw_teardown()
{
    teardown_item_particle();
}

void draw()
{
    assert(item_particle_list != NULL);
    item_particle_list->draw();
}

ItemParticle* create_item_particle(
    int particle_id, int item_type,
    float x, float y, float z, 
    float vx, float vy, float vz
) {
    ItemParticle* ip = item_particle_list->create(particle_id);
    if (ip == NULL) return NULL; 
    ip->init(item_type, x,y,z,vx,vy,vz);
    return ip;
}
#endif

#if DC_SERVER
ItemParticle* create_item_particle(
    ItemID item_id, int item_type,
    float x, float y, float z, 
    float vx, float vy, float vz
) {
    ItemParticle* ip = item_particle_list->create();
    if (ip == NULL) return NULL; 
    ip->init(item_id, item_type, x,y,z,vx,vy,vz);
    return ip;
}

// create Item and ItemParticle
Item::Item* create_item_particle(int item_type, float x, float y, float z, float vx, float vy, float vz)
{    
    Item::Item* item = Item::create_item(item_type);
    if (item == NULL) return NULL;
    ItemParticle* particle = create_item_particle(item->id, item->type, x,y,z,vx,vy,vz);
    if (particle == NULL) return item;
    broadcast_particle_item_create(particle->id);
    return item;
}

void broadcast_particle_item_create(int particle_id)
{
    ItemParticle* particle = item_particle_list->get(particle_id);
    assert(particle != NULL);

    item_particle_create_StoC msg;
    msg.id = particle->id;
    msg.item_type = particle->item_type;
    msg.x = particle->verlet.position.x;
    msg.y = particle->verlet.position.y;
    msg.z = particle->verlet.position.z;
    msg.mx = particle->verlet.velocity.x;
    msg.my = particle->verlet.velocity.y;
    msg.mz = particle->verlet.velocity.z;
    msg.broadcast();
}

void check_item_pickups()
{
    for (int i=0; i<item_particle_list->n_max; i++)
    {
        if (item_particle_list->a[i] == NULL) continue;
        ItemParticle* item_particle = item_particle_list->a[i];
        if (!item_particle->can_be_picked_up()) continue;
        Item::Item* item = Item::get_item(item_particle->item_id);
        assert(item != NULL);
    
        const static float pick_up_distance = 1.0f;
        Agent_state* agent = nearest_living_agent_in_range(item_particle->verlet.position, pick_up_distance);
        if (agent == NULL) continue;

        int container_id = Item::get_agent_container(agent->id);
        if (container_id == NULL_CONTAINER) return;

        // get slot for placing in container
        int slot = Item::auto_add_item_to_container(agent->client_id, container_id, item->id);   //insert item on server
        if (slot == NULL_SLOT) return;

        // update particle
        item_particle->picked_up(agent->id);
    }
}

void throw_item(int agent_id, ItemID item_id)
{
    printf("throw\n");
    assert(item_id != NULL_ITEM);
    Agent_state* a = ServerState::agent_list->get(agent_id);
    if (a == NULL) return;

    Item::Item* item = Item::get_item(item_id);
    if (item == NULL) return;

    Item::broadcast_item_destroy(item->id);

    Vec3 position = a->get_center();
    float x = position.x;
    float y = position.y;
    float z = position.z;

    const float mom = 2.0f;
    Vec3 force = a->s.forward_vector();
    force.z = 0;
    normalize_vector(&force);
    force = vec3_scalar_mult(force, mom);
    force = vec3_bias(force, (randf()-0.5f) * 30);
    float vx = force.x;
    float vy = force.y;
    float vz = force.z;

    // create particle
    ItemParticle* particle = create_item_particle(item->id, item->type, x,y,z,vx,vy,vz);
    if (particle == NULL) return;
    broadcast_particle_item_create(particle->id);
    particle->lock_pickup();
}


#endif

}   // ItemParticle