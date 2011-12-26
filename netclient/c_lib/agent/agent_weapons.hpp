#pragma once

#include <c_lib/weapons/weapons.hpp>

class Agent_state;  // forward declaration

class Agent_weapons {

    public:
        Agent_state* a;

        int active;

        Weapons::HitscanLaser laser;
        Weapons::BlockPick pick;
        Weapons::BlockApplier blocks;
        Weapons::GrenadeThrower grenades;

        void set_clip(int type, int clip);
        void set_ammo(int type, int ammo);
        void set_active(int slot);
        bool is_active(int type);
        int active_type();
        void set_active_block(int block);
        //void adjust_active();

        char* hud_display();

        bool fire();
        void reload();
        void reload(int type);

        Agent_weapons(Agent_state* a);
};
