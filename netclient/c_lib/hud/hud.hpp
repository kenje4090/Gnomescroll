#pragma once

#include <c_lib/hud/text.hpp>


namespace Hud
{

class ChatMessageQueue
{
};

class Scoreboard
{
};

using namespace HudText;

class HUD
{
    private:
        bool inited;
    
    public:
    
    // text objects
    Text* player_stats;
    Text* help;
    Text* disconnected;
    Text* dead;
    Text* fps;
    Text* ping;

    // scoreboard needs rewritten logic
    // will be its own class, also holding text objects
    Scoreboard* scoreboard;
    
    // chat queue
    ChatMessageQueue* chat_queue;    // contains text objects

    void init();
    HUD();
    ~HUD();
};

extern HUD* hud;

void init();

void set_hud_draw_settings(
    bool zoom,
    bool cube_selector,
    bool inventory,
    bool chat_cursor,
    bool help,
    bool disconnected,
    bool dead,
    bool fps,
    float fps_val,
    bool ping,
    int ping_val
);
void set_chat_cursor(char* text, float x, float y);

void draw_hud_textures();
void draw_hud_text();


}
