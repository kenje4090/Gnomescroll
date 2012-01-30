#pragma once

#include <c_lib/voxel/voxel_model.hpp>
#include <c_lib/voxel/voxel_dat.hpp>
#include <c_lib/common/enum_types.hpp>

extern VoxDat spawner_vox_dat;

const int SPAWNER_RADIUS = 5;
const int MAX_SPAWNERS = 18;
const int SPAWNERS_PER_TEAM = 9;

class Spawner_create_StoC; // forward decl

class Spawner
{
    public:
        int id;
        int team;
        int owner;
        Object_types type;

        float x,y,z;
        //float vx,vy,vz;
        float theta,phi;

        int spawn_radius;

        Voxel_model* vox;
        void init_vox();
        void set_team(int team);
        void set_owner(int owner);
        
        void get_spawn_point(int spawn[3], int agent_height=1);


        void create_message(Spawner_create_StoC* msg);
        Spawner(int id);
        Spawner(int id, float x, float y, float z);
        ~Spawner();
};

#include <c_lib/template/object_list.hpp>
class Spawner_list: public Object_list<Spawner,MAX_SPAWNERS>
{
    private:
        const char* name() { return "Spawner"; }

    public:
        bool team_spawner_available(int team);
        bool point_occupied(int x, int y, int z);
        void send_to_client(int client_id);
};

