#pragma once

#include <stdint.h>
#include <stddef.h>

typedef uint32_t ecs_component_mask_t;
typedef uint32_t ecs_id_t; // generic id type, will be used for systems

#define __ECS_MAX_SYSTEMS 8

enum ecs_err {
    ECS_OK = 0,
    ECS_FAIL,
    ECS_INVALID_ARGUMENT,
    ECS_ALLOC_FAILURE,
};

struct ecs_ctx {
    unsigned int components_count; // should be constant after initalization

    struct {
        size_t size;
        void* array;
        volatile unsigned int count;
    } entities;

    struct {
        unsigned int count;
        void (*pointers[__ECS_MAX_SYSTEMS])(void* entity);
    } systems;
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
