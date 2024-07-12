//===========================================================================
// EntityManager functions
//===========================================================================
EntityManager entity_manager_create(i32 max_entity_count) {
    EntityManager em = {};
    em.max_count = max_entity_count;
    em.entities = (Entity *)malloc(sizeof(Entity) * em.max_count);
    memset(em.entities, 0, sizeof(Entity) * em.max_count);
    em.first = 0;
    em.count = 0;
    em.free = em.entities;
    // initialize the free list
    for(i32 i = 0; i < em.max_count; i++) {
        Entity *entity = em.entities + i;
        if(i < (em.max_count - 1)) {
            entity->next = em.entities + (i + 1);
        }
        else {
            entity->next = 0;
        }
        if(i == 0) {
            entity->prev = 0;
        }
        else {
            entity->prev = em.entities + (i - 1);
        }
    }

    return em;
}

void entity_manager_destroy(EntityManager *em) {
    // free the entities array and set everything to zero
    free(em->entities);
    *em = {};
}

Entity *entity_manager_add_entity(EntityManager *em) {
    if(em->free == 0) {
        return 0;
    }

    Entity *entity = em->free;
    em->free = entity->next;

    if(em->first) {
        em->first->prev = entity;
    }
    entity->next = em->first;
    entity->prev = 0;
    em->first = entity;
    em->count++;

    return entity;
}

void entity_manager_remove_entity(EntityManager *em, Entity *entity) {
    if(em->first == entity) {
        em->first = entity->next;
    }
    if(entity->next) {
        entity->next->prev = entity->prev;
    }
    if(entity->prev) {
        entity->prev->next = entity->next;
    }
    entity->next = em->free;
    em->free = entity;
    em->count--;
}

void entity_manager_clear(EntityManager *em) {
    em->first = 0;
    em->count = 0;
    em->free = em->entities;
    // initialize the free list
    for(i32 i = 0; i < em->max_count; i++) {
        Entity *entity = em->entities + i;
        if(i < (em->max_count - 1)) {
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
//===========================================================================
//===========================================================================


