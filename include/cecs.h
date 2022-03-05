#pragma once

#include <stdint.h>
#include <stddef.h>

typedef uint32_t component_mask_t;

struct ecs_ctx {
    size_t entity_size;

    unsigned int comopnents_count; // should be constant after initalization

    void* entities;
    volatile unsigned int entities_count;
};

enum ecs_err {
    ECS_OK = 0,
    ECS_FAIL,
    ECS_INVALID_ARGUMENT,
    ECS_ALLOC_FAILURE,
};

const char* ecs_get_error();

enum ecs_err ecs_init(struct ecs_ctx* ctx, unsigned int components_count, size_t entity_size);

void ecs_cleanup(struct ecs_ctx* ctx);
