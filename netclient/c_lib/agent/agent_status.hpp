#pragma once

#define AGENT_HEALTH 100
#define RESPAWN_TICKS (2000 / 30)

class Agent_state;  // forward declaration
class PlayerAgent_state;

class Base_status {
    private:
    public:
        int health;
        bool dead;
        int respawn_countdown;
        
        Base_status() : health(AGENT_HEALTH), dead(false), respawn_countdown(RESPAWN_TICKS) {}
};

// Use for:
// All agents server side
// All non-player agents client side.
// In client side, ignore properties such as health
class Agent_status: public Base_status {

    private:
        Agent_state* a;
        void get_spawn_point(int* spawn);

    public:
        Agent_status(Agent_state* agent) : Base_status(), a(agent) {}

        int apply_damage(int dmg);
        void die();
        void respawn();

};

// Use for:
// Client side PlayerAgent
class PlayerAgent_status: public Base_status {

    private:
        PlayerAgent_state* a;

    public:
        PlayerAgent_status(PlayerAgent_state* agent) : Base_status(), a(agent) {}
};
