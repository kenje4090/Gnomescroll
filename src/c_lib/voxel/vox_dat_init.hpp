#pragma once

namespace VoxDats
{

// agent
extern Voxels::VoxDat agent;
extern Voxels::VoxDat agent_crouched;
extern Voxels::VoxDat agent_dead;

// monsters
extern Voxels::VoxDat robot_bomb;
extern Voxels::VoxDat robot_box;
extern Voxels::VoxDat monster_spawner;

// items
extern Voxels::VoxDat base;
extern Voxels::VoxDat energy_core;
extern Voxels::VoxDat agent_spawner;
extern Voxels::VoxDat turret;

void init();

} // VoxDats
