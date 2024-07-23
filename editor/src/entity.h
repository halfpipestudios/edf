#ifndef ENTITY_H
#define ENTITY_H

#define bit(value) (1 << value)

#define ENTITY_RENDER_COMPONENT    bit(0)
#define ENTITY_INPUT_COMPONENT     bit(1)
#define ENTITY_PHYSICS_COMPONENT   bit(2)
#define ENTITY_COLLISION_COMPONENT bit(3)
#define ENTITY_ASTEROID_COMPONENT  bit(4)
#define ENTITY_ENEMY0_COMPONENT    bit(5)
#define ENTITY_ENEMY1_COMPONENT    bit(6)
#define ENTITY_ENEMY2_COMPONENT    bit(7)
#define ENTITY_TRIGGER_COMPONENT   bit(8)
#define ENTITY_ANIMATION_COMPONENT bit(9)

static const char *component_strings[] {
    "Render Component",
    "Input Component",
    "Physics Component",
    "Collision Component",
    "Asteroid Component",
    "Enemy0 Component",
    "Enemy1 Component",
    "Enemy2 Component",
    "Trigger Component",
    "Animation Component"
};

static const char *collision_type_strings[] {
    "Circle Collision",
    "AABB Collision",
    "OBB Collision"
};

enum CollisionType {
    COLLISION_TYPE_CIRLCE,
    COLLISION_TYPE_AABB,
    COLLISION_TYPE_OBB
};

struct Collision {
    CollisionType type; 
    union {
        Circle circle;
        AABB   aabb;
        OBB    obb;
    };
    V2 offset;
};

struct Texture {
    i32 w, h;
    u32 format;
    SDL_Texture *texture;
    SDL_Texture *mask;
};

struct Entity {
    u64 components;
    V2 pos;
    V2 scale;
    f32 angle;
    V4 tint;
    Texture texture;

    V2 vel;
    V2 acc;
    f32 damping;

    Collision collision;
    bool collides;

    Entity *next;
    Entity *prev;
    u32 uid;
};

struct EntityManager {
    Entity *entities;
    Entity *first;
    Entity *free;
    i32 count;
    i32 max_count;
};

#endif // ENTITY_H
