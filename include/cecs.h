#pragma once

#include <stdint.h>
#include <stddef.h>

typedef uint32_t component_mask_t;

struct ecs_ctx {
    size_t entity_size;

    unsigned int components_count; // should be constant after initalization

    void* entities;
    volatile unsigned int entities_count;
};

enum ecs_err {
    ECS_OK = 0,
    ECS_FAIL,
    ECS_INVALID_ARGUMENT,
    ECS_ALLOC_FAILURE,
};

// Context initilization/desctruction
// ecs_init will use config structure instead of a very long list of arguments
// benefits: won't need to change function signature that often
//      looks cleaner on both sides(?)

struct ecs_config {
    unsigned int components_count;
    size_t entity_size;
};

enum ecs_err    ecs_init(struct ecs_ctx* ctx, struct ecs_config* config);
void            ecs_cleanup(struct ecs_ctx* ctx);

// Active context {get,set}ter
void            ecs_set_ctx(struct ecs_ctx* ctx);
struct ecs_ctx* ecs_get_ctx();

const char*     ecs_get_error();
