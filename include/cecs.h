#pragma once

#include <stdint.h>
#include <stddef.h>

typedef uint32_t ecs_component_mask_t;
typedef uint32_t ecs_id_t; // generic id type, will be used for systems

#ifdef DEBUG
#define DBGMSG(msg...) do { printf(msg); } while (0)
#else
#define DBGMSG(msg...) do {} while (0)
#endif

enum ecs_err {
    ECS_OK = 0,
    ECS_FAIL,
    ECS_INVALID_ARGUMENT,
    ECS_ALLOC_FAILURE,
};

struct ecs_entity {
    ecs_component_mask_t component_mask;
};

struct ecs_component_pool {
    ecs_component_mask_t mask;
    size_t component_size;
    void* pool;
};

// Context initilization/desctruction
// ecs_init will use config structure instead of a very long list of arguments
// benefits: won't need to change function signature that often
//      looks cleaner on both sides(?)

struct ecs_config {
    size_t systems_pool_size;
    size_t entities_pool_size;

    size_t components_pool_pool_size;
};

// both component and entity id will be their index in their pools
// and systems i think can just have id as an index too
// so component doesn't have to have owner_id or something


// i think that we're not going to use 0'th index, so we can return 0 as an error
struct ecs_ctx {
    struct ecs_config config;

    struct ecs_entity* entities;

    struct ecs_system {
        ecs_component_mask_t component_mask;

        void (*function)(struct ecs_ctx* ctx, size_t entity_id);
    } *systems;

    struct {
        size_t count;
        struct ecs_component_pool* pools;
    } components;
};

enum ecs_err    ecs_init(struct ecs_ctx* ctx, struct ecs_config* config);
void            ecs_cleanup(struct ecs_ctx* ctx);

const char*     ecs_get_error();

size_t          ecs_register_system(struct ecs_ctx* ctx, struct ecs_system* system);
void            ecs_remove_system(struct ecs_ctx* ctx, size_t id);

enum ecs_err    ecs_register_component(struct ecs_ctx* ctx, ecs_component_mask_t mask, size_t size);
enum ecs_err    ecs_remove_component(struct ecs_ctx* ctx, ecs_component_mask_t mask);

size_t          ecs_add_entity(struct ecs_ctx* ctx, ecs_component_mask_t component_mask);

enum ecs_err    ecs_run(struct ecs_ctx* ctx);

void*           ecs_get_component(struct ecs_ctx* ctx, ecs_component_mask_t mask, size_t id);
