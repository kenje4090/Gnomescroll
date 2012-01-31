#pragma once

#include <c_lib/game/game.hpp>
#include <c_lib/game/teams.hpp>

class CTF {

    public:

        #ifdef DC_CLIENT
        bool auto_assign;
        void join_team(int team);
        void on_ready();

        void flag_picked_up(int team);
        void flag_dropped(int team);
        void flag_scored(int team);

        int get_team_color(int team, unsigned char *r, unsigned char *g, unsigned char *b);
        void animate_flags();
        #endif

        #ifdef DC_SERVER
        void on_client_connect(int client_id);
        void auto_assign_agent(int agent_id);
        void send_to_client(int client_id);

        void reset_flag(int team);
        void check_agent_proximities();        
        void agent_drop_flag(int agent_team, float x, float y, float z);
        bool remove_agent_from_team(int agent);
        #endif

        NoTeam none;
        CTFTeam one;
        CTFTeam two;

        void init();
        void start();
        void tick();

        void set_team_name(int team, char* name);
        
        void set_score(int team, int score);
        void set_flag_position(int team, float x, float y, float z);
        void set_base_position(int team, float x, float y, float z);

        void get_base_spawn_point(int team, int agent_height, int* spawn);

        bool add_agent_to_team(int team, int agent);
        void set_team_color(int team,
            unsigned char r, unsigned char g, unsigned char b);

        CTF();
};

