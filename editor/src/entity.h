#ifndef ENTITY_H
#define ENTITY_H

struct Texture {
    i32 w, h;
    u32 format;
    SDL_Texture *texture;
    SDL_Texture *mask;
};

struct Entity {
    u32 uid;
    V2 pos;
    V2 scale;
    f32 angle;
    Texture texture;

    Entity *next;
    Entity *prev;
};

struct EntityManager {
    Entity *entities;
    Entity *first;
    Entity *free;
    i32 count;
    i32 max_count;
};

#endif // ENTITY_H
