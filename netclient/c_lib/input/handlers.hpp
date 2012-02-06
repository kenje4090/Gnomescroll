#pragma once

typedef enum InputStateMode
{
    INPUT_STATE_AGENT,
    INPUT_STATE_CAMERA
} InputStateMode;

struct InputState
{
    bool mouse_bound;
    bool help_menu;
    bool inventory;
    bool scoreboard;
    bool map;
    bool chat;
    bool hud;
    bool debug;
    
    bool can_jump;
    bool quit;

    InputStateMode input_mode;
    InputStateMode camera_mode;

    // options
    bool invert_mouse;
    float sensitivity;
};

extern InputState input_state;

void init_handlers();

// handlers
void key_down_handler(SDL_Event* event);
void key_up_handler(SDL_Event* event);
void mouse_button_down_handler(SDL_Event* event);
void mouse_button_up_handler(SDL_Event* event);
void mouse_motion_handler(SDL_Event* event);
void quit_event_handler(SDL_Event* event);
void key_state_handler(Uint8 *keystate, int numkeys);

// triggers
void toggle_mouse_bind();
void toggle_help_menu();
void toggle_inventory();
void toggle_scoreboard();
void toggle_map();
void toggle_chat();

void enable_jump();
void enable_quit();

void toggle_input_mode();
void toggle_camera_mode();

//chat
extern const int CHAT_BUFFER_SIZE;
extern int* chat_input_buffer_unicode;
extern char** chat_input_buffer_sym;
extern int chat_cursor_index;
void clear_chat_buffer();

// options
void set_input_options(
    bool invert_mouse,
    float sensitivity
);
