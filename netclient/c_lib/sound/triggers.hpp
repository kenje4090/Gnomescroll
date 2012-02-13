#pragma once

namespace Sound
{

void fire_laser();
void fire_laser(float x, float y, float z, float vx, float vy, float vz);
void out_of_ammo();
void out_of_ammo(float x, float y, float z, float vx, float vy, float vz);
void reload();
void reload(float x, float y, float z, float vx, float vy, float vz);
void laser_hit_block(float x, float y, float z, float vx, float vy, float vz);
void laser_hit_agent(float x, float y, float z, float vx, float vy, float vz);
void pick_hit_block(float x, float y, float z, float vx, float vy, float vz);
void pick_hit_agent(float x, float y, float z, float vx, float vy, float vz);
void pick_swung();
void pick_swung(float x, float y, float z, float vx, float vy, float vz);
void block_destroyed(float x, float y, float z, float vx, float vy, float vz);
void block_took_damage(float x, float y, float z, float vx, float vy, float vz);
void flag_picked_up();
void flag_scored();
void spawner_placed(float x, float y, float z, float vx, float vy, float vz);
void respawned();
void died();
void died(float x, float y, float z, float vx, float vy, float vz);
void walking();
void running();
void jetpack_on();
void walking(float x, float y, float z, float vx, float vy, float vz);
void running(float x, float y, float z, float vx, float vy, float vz);
void jetpack_on(float x, float y, float z, float vx, float vy, float vz);
void soft_landing();
void medium_landing();
void hard_landing();
void soft_landing(float x, float y, float z, float vx, float vy, float vz);
void medium_landing(float x, float y, float z, float vx, float vy, float vz);
void hard_landing(float x, float y, float z, float vx, float vy, float vz);
void victory();
void restore_health();
void restore_ammo();
void restore_health(float x, float y, float z, float vx, float vy, float vz);
void restore_ammo(float x, float y, float z, float vx, float vy, float vz);
void block_set(float x, float y, float z, float vx, float vy, float vz);
void grenade_explode(float x, float y, float z, float vx, float vy, float vz);
void grenade_bounce(float x, float y, float z, float vx, float vy, float vz);
void agent_took_damage();
void agent_took_damage(float x, float y, float z, float vx, float vy, float vz);

}
