//
//  edf_entity.h
//  edf
//
//  Created by Manuel Cabrerizo on 24/06/2024.
//

#ifndef EDF_ENTITY_H
#define EDF_ENTITY_H

#include "edf_common.h"
#include "edf_math.h"
#include "edf_platform.h"
#include "edf_collision.h"

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

#define ENTITY_MANAGER_MAX_ENTITIES 1000

struct GameState;
struct Arena;

typedef enum CollisionType {
    COLLISION_TYPE_CIRLCE,
    COLLISION_TYPE_AABB,
    COLLISION_TYPE_OBB
} CollisionType;

typedef struct Collision {
    CollisionType type; 
    union {
        Circle circle;
        AABB   aabb;
        OBB    obb;
    };
} Collision;

typedef struct Entity {
    u64 components;
    V3 pos;
    V3 save_pos;
    V2 vel;
    V2 acc;
    V2 scale;
    f32 angle;
    f32 damping;
    V4 tint;
    
    Texture tex;
    Texture save_tex;
    Animation *animation;

    // collision
    Collision collision;
    bool collides;
    
    // triggers
    bool active;
    struct Entity *to_trigger;
    i32 to_trigger_count;

// internal data for the entity manager
    struct Entity *next;
    struct Entity *prev;
} Entity;

void entity_add_render_component(Entity *entity, V3 pos, V2 scale, Texture texture, V4 tint);
void entity_add_input_component(Entity *entity);
void entity_add_physics_component(Entity *entity, V2 vel, V2 acc, f32 damping);
void entity_add_collision_component(Entity *entity, Collision collision, bool collides);
void entity_add_asteroid_component(Entity *entity, V2 vel, Entity *trigger);
void entity_add_enemy0_component(Entity *entity); // TODO:...
void entity_add_enemy1_component(Entity *entity); // TODO:...
void entity_add_enemy2_component(Entity *entity); // TODO:...
void entity_add_trigger_component(Entity *entity);
void entity_add_animation_component(Entity *entity, Animation *animation);

void entity_remove_components(Entity *entity, u64 components);

#define SYSTEM_UPDATE(name) void name(struct GameState *gs, Entity *entity, Entity **others, i32 others_count, f32 dt)
typedef SYSTEM_UPDATE(SystemUpdateFunc);

typedef struct EntityManager {
    Entity entities[ENTITY_MANAGER_MAX_ENTITIES];
    Entity *first;
    Entity *free_list;
    i32 entity_count;
    i32 entity_max_cout;
} EntityManager;

EntityManager *entity_manager_load(struct Arena *arena, i32 entity_max_count);
Entity *entity_manager_add_entity(EntityManager *em);
void entity_manager_remove_entity(EntityManager *em, Entity *entity);
void entity_manager_clear(EntityManager *em);
void entity_manager_forall(struct GameState *gs, EntityManager *em, SystemUpdateFunc *system_update, u64 components, f32 dt);


#endif /* EDF_ENTITY_H */
