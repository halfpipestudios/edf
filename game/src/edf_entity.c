//
//  edf_entity.c
//  edf
//
//  Created by Manuel Cabrerizo on 24/06/2024.
//

#include "edf_entity.h"
#include "edf_memory.h"

void entity_add_render_component(Entity *entity, V3 pos, V2 scale, Texture texture, V4 tint) {
    entity->components |= ENTITY_RENDER_COMPONENT;
    entity->pos = pos;
    entity->scale = scale;
    entity->tex = texture;
    entity->tint = tint;
}

void entity_add_input_component(Entity *entity) {
    entity->components |= ENTITY_INPUT_COMPONENT;
}

void entity_add_physics_component(Entity *entity, V2 vel, V2 acc, f32 damping) {
    entity->components |= ENTITY_PHYSICS_COMPONENT;
    entity->vel = vel;
    entity->acc = acc;
    entity->damping = damping;
}

void entity_add_collision_component(Entity *entity, Collision collision, bool collides) {
    entity->components |= ENTITY_COLLISION_COMPONENT;
    entity->collision = collision;
    entity->collides = collides;
}

void entity_add_asteroid_component(Entity *entity, V2 vel, Entity *trigger) {
    assert((trigger->components & ENTITY_TRIGGER_COMPONENT) != 0);
    entity->components |= ENTITY_ASTEROID_COMPONENT;
    entity->vel = vel;
    entity->active = false;
    Entity *last = trigger;
    while(last->to_trigger) {
        last = last->to_trigger; 
    }
    last->to_trigger = entity;
    trigger->to_trigger_count++;
}

void entity_add_enemy0_component(Entity *entity) {
    entity->components |= ENTITY_ENEMY0_COMPONENT;
}

void entity_add_enemy1_component(Entity *entity) {
    entity->components |= ENTITY_ENEMY1_COMPONENT;
}

void entity_add_enemy2_component(Entity *entity) {
    entity->components |= ENTITY_ENEMY2_COMPONENT;
}

void entity_add_trigger_component(Entity *entity) {
    assert((entity->components & ENTITY_COLLISION_COMPONENT) != 0);
    entity->components |= ENTITY_TRIGGER_COMPONENT;
    entity->to_trigger = 0;
    entity->to_trigger_count = 0;
}


void entity_add_animation_component(Entity *entity, Animation *animation) {
    entity->save_tex = entity->tex;
    entity->components |= ENTITY_ANIMATION_COMPONENT;
    entity->animation = animation;
    return;
}

void entity_remove_components(Entity *entity, u64 components) {
    entity->components &= ~components;
}

EntityManager *entity_manager_load(Arena *arena, i32 entity_max_count) {
    // Init the entity manager
    EntityManager *em = (EntityManager *)arena_push(arena, sizeof(EntityManager), 8);
    em->entity_count = 0;
    em->entity_max_cout = entity_max_count;
    em->first = 0;
    em->free_list = em->entities;
    for(i32 i = 0; i < em->entity_max_cout; i++) {
        Entity *entity = em->entities + i;
        memset(entity, 0, sizeof(Entity));
        if(i < (em->entity_max_cout - 1)) {
            entity->next = em->entities + (i + 1);
        }
        else {
            entity->next = 0;
        }
        if(i == 0) {
            entity->prev = 0;
        }
        else {
            entity->prev = em->entities + (i - 1);
        }
    }
    return em;    
}

Entity *entity_manager_add_entity(EntityManager *em) {
    if(em->free_list){
        // get a new entity and update the free list
        Entity *entity = em->free_list;
        em->free_list = em->free_list->next; 
        // init the new entity
        memset(entity, 0, sizeof(Entity));
        entity->next = em->first;
        if(em->first) {
            em->first->prev = entity;
        }
        em->first = entity;
        em->entity_count++;
        return entity;
    }
    return 0;
}


void entity_manager_remove_entity(EntityManager *em, Entity *entity) {
    if(entity) {
        if(entity->prev) {
            entity->prev->next = entity->next;
        }
        if(entity->next) {
            entity->next->prev = entity->prev;
        }
        entity->next = em->free_list;
        entity->prev = 0;
        em->free_list = entity;
        em->entity_count--;
    }
}

void entity_manager_clear(EntityManager *em) {
    em->entity_count = 0;
    em->first = 0;
    em->free_list = em->entities;
    for(i32 i = 0; i < em->entity_max_cout; i++) {
        Entity *entity = em->entities + i;
        memset(entity, 0, sizeof(Entity));
        if(i < (em->entity_max_cout - 1)) {
            entity->next = em->entities + (i + 1);
        }
        else {
            entity->next = 0;
        }
        if(i == 0) {
            entity->prev = 0;
        }
        else {
            entity->prev = em->entities + (i - 1);
        }
    }
}


void entity_manager_forall(struct GameState *gs, EntityManager *em, SystemUpdateFunc *system_update, u64 components, f32 dt) {
    i32 entity_to_update_count = 0;
    Entity *entities_to_update[ENTITY_MANAGER_MAX_ENTITIES];

    Entity *entity = em->first;
    while(entity) {
        if((entity->components & components) == components)  {
            entities_to_update[entity_to_update_count] = entity;
            entity_to_update_count++;
        }
        entity = entity->next;
    }

    for(i32 i = 0; i < entity_to_update_count; i++) {
        Entity *entity = entities_to_update[i];
        system_update(gs, entity, entities_to_update, entity_to_update_count, dt);
    }
}
