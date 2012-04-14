#include "client_state.hpp"

#if DC_CLIENT

#include <c_lib/agent/agent_list.hpp>
#include <c_lib/game/ctf.hpp>
#include <c_lib/voxel/voxel_render.hpp>

//#include <c_lib/animations/hitscan.hpp>
//#include <c_lib/animations/hitscan_laser.hpp>
#include <c_lib/hud/text.hpp>

#include <c_lib/agent/agent.hpp>
#include <c_lib/agent/net_agent.hpp>

#include <c_lib/agent/client/player_agent.hpp>

#include <c_lib/particles/_interface.hpp>

#if USE_OPENAL
#include <c_lib/sound/openal.hpp>
#endif

const int GAME_OBJECT_MAX = 4096 * 4;

namespace ClientState {

    //using Animations::HitscanEffect_list;
    //using Animations::HitscanLaserEffect_list;

    class Agent_list* agent_list = NULL;

    class Voxel_render_list* voxel_render_list;
    class Voxel_hitscan_list* voxel_hitscan_list = NULL;
    class SpawnerList* spawner_list = NULL;
    class OwnedList* owned_list = NULL;

    class GameObject_list* object_list;

    char desired_name[PLAYER_NAME_MAX_LENGTH+1];
    int last_ping_time;
    int last_reliable_ping_time;

    PlayerAgent_state playerAgent_state;

    int agent_control_state[16];

    CTF* ctf = NULL;
        
    void init_lists()
    {
        voxel_render_list = new Voxel_render_list;
        voxel_hitscan_list = new Voxel_hitscan_list;
        spawner_list = new SpawnerList;
        owned_list = new OwnedList;
        
        object_list = new GameObject_list;
        agent_list = new Agent_list;
    }

    void teardown_lists()
    {
        // voxel models
        delete agent_list;
        delete object_list;

        // voxel lists
        // must go after all voxels
        delete voxel_render_list;
        delete voxel_hitscan_list;

        // behaviour lists
        delete spawner_list;
        delete owned_list;
    }

    static void init_ctf()
    {
        static int inited = 0;
        if (inited++)
        {
            printf("WARNING: ClientState::init_ctf -- attempt to call more than once\n");
            return;
        }
        ctf = new CTF;
        ctf->init();
    }
    
    static void teardown_ctf()
    {
        if (ctf != NULL)
            delete ctf;
    }

    void init()
    {
        static int inited = 0;
        if (inited++)
        {
            printf("WARNING: ClientState::init -- attempt to call more than once\n");
            return;
        }
        init_lists();
        voxel_render_list->init_voxel_render_list_shader1();
    }

    void teardown()
    {
        teardown_ctf();
        teardown_lists();
    }

    void update()
    {
        ctf->update();
        //object_list->update();

        if (playerAgent_state.you != NULL)
            HudMap::update_team(playerAgent_state.you->status.team);
        static int team_color_update = 30 * 5;  // start at 4seconds in, so initial update happens quickly
        const int team_color_update_interval = 30 * 6;  // once every 6 seconds
        if (team_color_update++ >= team_color_update_interval)
        {
            agent_list->update_team_colors();
            Compass::update_team_colors();
            team_color_update %= team_color_update_interval;
        }

        // update hud projected names
        ClientState::playerAgent_state.display_agent_names();   
        // check if we've failed to receive any identify packets (lost in initialization)
        // Shouldn't be needed? initialization packets are reliable
        ClientState::agent_list->check_missing_names();  // will ratelimit itself
        ClientState::agent_list->check_if_at_base();  // will ratelimit itself

    }

    void set_PlayerAgent_id(int id)
    {
        playerAgent_state.set_PlayerAgent_id(id);
        ctf->on_ready();
    }

    void tick()
    {
        Particles::grenade_list->tick();
        Particles::shrapnel_list->tick();
        Particles::blood_list->tick();
        Particles::colored_minivox_list->tick();
        Particles::textured_minivox_list->tick();
        Particles::billboard_text_list->tick();
        Particles::billboard_text_hud_list->tick();

        object_list->tick();
    }

    void draw()
    {
        // voxels
        agent_list->update_models();
        object_list->update();  // model updates
        voxel_render_list->draw();

        // quads
        glColor3ub(255,255,255);
        GL_ASSERT(GL_DEPTH_TEST, true);
        glBegin(GL_QUADS);

        Particles::colored_minivox_list->draw();
        Draw::colored_minivox_list->draw();   // new entity system registries

        glEnd();

        glColor3ub(255,255,255);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, t_map::block_textures_normal);
        glBegin(GL_QUADS);

        Particles::textured_minivox_list->draw();
        Draw::textured_minivox_list->draw();

        glEnd();
        glDisable(GL_TEXTURE_2D);

        // transparent
        Particles::billboard_text_list->draw();        
    }

    void send_identify_packet(char* name)
    {
        unsigned int len = strlen(name);
        if (len >= PLAYER_NAME_MAX_LENGTH)
            name[PLAYER_NAME_MAX_LENGTH-1] = '\0';

        len = sanitize_player_name(name);
        if (len == 0)
            return;
        
        printf("sending identify packet to server; name=%s\n", name);
        identify_CtoS msg;
        strcpy(msg.name, name);
        msg.send();
    }
    
    void update_camera()
    {
        if (input_state.camera_mode == INPUT_STATE_AGENT)
            use_agent_camera();
        else
            use_free_camera();

        update_agent_camera();
    }

    void send_ping()
    {
        int n = _GET_MS_TIME();
        
        ping_CtoS msg;
        msg.ticks = n;
        msg.send();

        ping_reliable_CtoS msg2;
        msg2.ticks = n;
        msg2.send();
    }

    void set_desired_name(char* name)
    {
        if (strlen(name) > PLAYER_NAME_MAX_LENGTH)
            name[PLAYER_NAME_MAX_LENGTH] = '\0';

        strcpy(desired_name, name);
    }

    void client_id_received(int client_id)
    {
        send_identify_packet(desired_name);
    }

    void on_connect()
    {
        chat_client->send_system_message((char*)"Connected to server");   
    }
    
    void on_disconnect()
    {
        chat_client->send_system_message((char*)"Disconnected from server");
    }
}

#endif
