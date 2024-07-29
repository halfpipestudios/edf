//===========================================================================
// Entity functions
//===========================================================================

static void entity_add_components(Entity *entity, u64 components) {
    entity->components |= components;
}

static void entity_remove_components(Entity *entity, u64 components) {
    entity->components &= ~components;
}

static void entity_write_to_buffer(EditorState *es, Entity *entity, EntitySerialized *e) {
    // componenst
    e->components = entity->components;
    // graphic component
    e->pos = entity->pos;
    e->scale = entity->scale;
    e->angle = entity->angle;
    e->tint = entity->tint;

    char *texture_name = es->textures_names[entity->texture.index];
    memcpy(e->texture, texture_name, strlen(texture_name));

    // physics component
    e->vel = entity->vel;
    e->acc = entity->acc;
    e->damping = entity->damping;
    // collision component
    e->collision = entity->collision;
    e->collides = entity->collides;
}

//===========================================================================
//===========================================================================

//===========================================================================
// EntityManager functions
//===========================================================================
static EntityManager entity_manager_create(i32 max_entity_count) {
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

static void entity_manager_destroy(EntityManager *em) {
    // free the entities array and set everything to zero
    free(em->entities);
    *em = {};
}

static Entity *entity_manager_add_entity(EntityManager *em) {
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

static void entity_manager_remove_entity(EntityManager *em, Entity *entity) {
    if(entity) {
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
}

static void entity_manager_clear(EntityManager *em) {
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


static void entity_manager_serialize(EntityManager *em, const char *path, EditorState *es) {
    size_t buffer_size = sizeof(EntitySerialized) * em->count + sizeof(i32);
    u8 *buffer = (u8 *)malloc(buffer_size);
    memset(buffer, 0, buffer_size);

    
    i32 *entity_count = (i32 *)buffer;
    EntitySerialized *entity_buffer = (EntitySerialized *)(buffer + sizeof(i32));

    *entity_count = em->count;

    i32 i = 0;
    Entity *entity = em->first;
    while(entity) {
        entity_write_to_buffer(es, entity, entity_buffer + i);
        entity = entity->next;
        i++;
    }

    FILE *file = fopen(path, "wb");
    if(file) {
        printf("Success open file: %s\n", path);
        fwrite(buffer, buffer_size, 1, file);
        fclose(file);
    }
    else {
        printf("Error open file: %s\n", path);
    }

    free(buffer);
}
//===========================================================================
//===========================================================================


