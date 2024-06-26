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

#define bit(value) (1 << value)

#define ENTITY_RENDER_COMPONENT  bit(0)
#define ENTITY_INPUT_COMPONENT   bit(1)
#define ENTITY_PHYSICS_COMPONENT bit(2)
#define ENTITY_AI_COMPONENT      bit(3)

#define ENTITY_MANAGER_MAX_ENTITIES 1000

struct GameState;

typedef struct Entity {
    u64 components;
    V3 pos;
    V2 vel;
    V2 acc;
    V2 scale;
    f32 angle;
    f32 damping;
    Texture tex;
    V4 tint;

// internal data for the entity manager
    struct Entity *next;
    struct Entity *prev;
} Entity;

void entity_add_render_component(Entity *entity, V3 pos, V2 scale, Texture texture, V4 tint);
void entity_add_input_component(Entity *entity);
void entity_add_physics_component(Entity *entity, V2 vel, V2 acc, f32 damping);
void entity_add_ai_component(Entity *entity);
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
