#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <cecs.h>
#include <error.h>

// user aren't supposed to modify ctx structure directly
// but only use documented functions
//
// in this realization we will be able to dynamicly
// register new components

enum ecs_err ecs_init(struct ecs_ctx* ctx, struct ecs_config* config) {
    memset(ctx, 0, sizeof(*ctx));

    ctx->config = *config;
    ctx->entities = calloc(config->entities_pool_size + 1, sizeof(struct ecs_entity));
    memset(ctx->entities, 0, (config->entities_pool_size + 1) * sizeof(struct ecs_entity));

    ctx->systems = calloc(config->systems_pool_size + 1, sizeof(struct ecs_system));
    memset(ctx->systems, 0, (config->systems_pool_size + 1) * sizeof(struct ecs_system));

    ctx->components.pools = calloc(config->components_pool_pool_size, sizeof(struct ecs_component_pool));
    memset(ctx->components.pools, 0, config->components_pool_pool_size * sizeof(struct ecs_component_pool));

    return ECS_OK;
}

void ecs_cleanup(struct ecs_ctx* ctx) {
    if (ctx->entities != NULL)
        free(ctx->entities);

    if (ctx->systems != NULL)
        free(ctx->systems);

    if (ctx->components.pools != NULL)
        free(ctx->components.pools);
}

size_t ecs_get_free_system_id(struct ecs_ctx* ctx) {
    for (size_t i = 1; i <= ctx->config.systems_pool_size; i++) {
        // systems that don't depend on components considered invalid
        if (ctx->systems[i].component_mask == 0) {
            DBGMSG("%s: found free system id (%d), returning\n", __func__, i);
            return i;
        }
    }
    return 0;
}

size_t ecs_register_system(struct ecs_ctx* ctx, struct ecs_system* system) {

    size_t id = ecs_get_free_system_id(ctx);
    if (id > 0) {
        DBGMSG("%s: registering system\n", __func__);
        ctx->systems[id] = *system;
    }

    return id;
}

void ecs_remove_system(struct ecs_ctx* ctx, size_t id) {
    ctx->systems[id].component_mask = 0;
}

size_t ecs_get_free_entity_id(struct ecs_ctx* ctx) {
    // not using 0'th index so we can treat 0 as an error
    for (size_t i = 1; i <= ctx->config.entities_pool_size; i++) {
        // entities with no components considered invalid
        if (ctx->entities[i].component_mask == 0)
            return i;
    }
    ecs_set_error("failed to get free entity id, propably no space in pool left\n");
    return 0;
}

size_t ecs_add_entity(struct ecs_ctx* ctx, ecs_component_mask_t component_mask) {
    size_t id = ecs_get_free_entity_id(ctx);
    if (id > 0) {
        ctx->entities[id].component_mask = component_mask;
    }
    return id;
}

enum ecs_err ecs_register_component(struct ecs_ctx* ctx, ecs_component_mask_t mask, size_t size) {
    // maybe add checking for duplicate component masks
    if (mask == 0) {
        ecs_set_error("component mask cannot be all zeros");
        return ECS_INVALID_ARGUMENT;
    }

    // TODO: try to find free before

    ctx->components.pools[ctx->components.count] = (struct ecs_component_pool){
        .pool = calloc(ctx->config.entities_pool_size, size),
        .mask = mask,
        .component_size = size,
    };

    ctx->components.count++;

    return ECS_OK;
}

enum ecs_err ecs_remove_component(struct ecs_ctx* ctx, ecs_component_mask_t mask) {
    if (mask == 0) {
        return ECS_INVALID_ARGUMENT;
    }
    for (size_t i = 0; i < ctx->components.count; i++) {
        struct ecs_component_pool* pool = ctx->components.pools + i;
        if (pool->mask == mask) {
            DBGMSG("freeing pool %p, that has mask %d\n", pool->pool, pool->mask);
            free(pool->pool);
            pool->mask = 0;

            return ECS_OK;
        } 
    }
    return ECS_FAIL;
}

struct ecs_component_pool* ecs_get_component_pool(struct ecs_ctx* ctx, ecs_component_mask_t mask) {
    for (size_t i = 0; i < ctx->components.count; i++) {
        if (mask & ctx->components.pools[i].mask) {
            return ctx->components.pools + i;
        }
    }
    // TODO: err msg
    return NULL;
}

void* ecs_get_component(struct ecs_ctx* ctx, ecs_component_mask_t mask, size_t id) {
    struct ecs_component_pool* pool = ecs_get_component_pool(ctx, mask);
    if (pool == NULL) return NULL;

    void* component = pool->pool + pool->component_size * id;

    return component;
}

void ecs_remove_entity(struct ecs_ctx* ctx, size_t id) {
    ctx->entities[id].component_mask = 0;
}

enum ecs_err ecs_run(struct ecs_ctx *ctx) {
    for (size_t system_id = 1; system_id <= ctx->config.systems_pool_size; system_id++) {
        // invalid system
        if (ctx->entities[system_id].component_mask == 0) continue;

        for (size_t entity_id = 1; entity_id <= ctx->config.entities_pool_size; entity_id++) {
            // invalid entity
            if (ctx->entities[entity_id].component_mask == 0) continue;

            if (ctx->systems[system_id].component_mask & ctx->entities[entity_id].component_mask) {
                ctx->systems[system_id].function(ctx, entity_id);
            }
        }
    }
    return ECS_OK;
}
