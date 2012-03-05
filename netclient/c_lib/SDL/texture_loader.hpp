#pragma once

#include <compat_gl.h>

struct Texture {
    int tex;
    int w;
    int h;
};

int init_image_loader();

SDL_Surface* _load_image(char *file);
struct Texture _load_image_create_texture(char *file);
void _load_image_create_texture(char *file, struct Texture* tex);

SDL_Surface* create_surface_from_file(char* file);
int create_texture_from_file(char* filename, int* tex);   // deprecated
int create_texture_from_file(char* filename, GLuint* tex);
int create_texture_from_surface(SDL_Surface* surface, int* tex);   // deprecated
int create_texture_from_surface(SDL_Surface* surface, GLuint* tex);
SDL_Surface* create_texture_and_surface_from_file(char* filename, GLuint* tex);

SDL_Surface* create_surface_from_nothing(int w, int h);

void load_colored_texture(
    char* path, GLuint* texture,
    unsigned char br, unsigned char bg, unsigned char bb,   // base color
    unsigned char r, unsigned char g, unsigned char b       // replace with
);
