#ifndef ENTITY_H
#define ENTITY_H

struct Entity {
    u32 uid;
    V2 pos;
    V2 scale;
    SDL_Texture *texture;

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
