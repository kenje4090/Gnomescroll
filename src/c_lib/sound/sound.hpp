#pragma once

#include <common/defines.h>
#include <sound/triggers.hpp>

namespace Sound {

void init();
void close();

void set_volume(float vol);

void load_sound(Soundfile* snd);

int play_2d_sound(char* file);
int play_3d_sound(char* file, float x, float y, float z, float vx, float vy, float vz);

void update_listener(float x, float y, float z, float vx, float vy, float vz, float fx, float fy, float fz, float ux, float uy, float uz);
void update();

int test();

//CYTHON
void enumerate_sound_devices() GNOMESCROLL_API;
}