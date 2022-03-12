#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <dbg.h>

#include <cecs.h>

#define __ECS_ERROR_MSG_BUF_SIZE 128

// later something like (sizeof(component_mask_t) * CHAR_BIT)
#define __ECS_MAX_COMPONENTS 8

#define __ECS_MAX_ENTITIES 8

// i think we should not use ctx structure directly
// but provide functions instead
//
// in this realization we won't be able to dynamicly
// register new components
//
// ecs_ctx should know only components count and the sum of their sizes
//
// components_count in ecs_ctx will only be used to compute their masks
// and then to query entites with specified components
//
// actual entity struct will be defined by user
// it must include component_mask_t as FIRST element
// AND all possible user defined components

// we should maybe make this just a pointer later,
// since error messages propably will only be string literals
static char error_msg_buf[__ECS_ERROR_MSG_BUF_SIZE];
static void ecs_set_error(const char* message) {
    strcpy(error_msg_buf, message);
}

const char* ecs_get_error() {
    return error_msg_buf;
}

enum ecs_err ecs_init(struct ecs_ctx* ctx, unsigned int components_count, size_t entity_size) {
    if (entity_size == 0) {
        ecs_set_error("ecs_init(): entity_size cannot be 0");
        return ECS_INVALID_ARGUMENT;
    }
    if (components_count < 1) {
        ecs_set_error("ecs_init(): There must be at least one component!");
        return ECS_INVALID_ARGUMENT;
    }
    if (components_count > __ECS_MAX_COMPONENTS) {
        char errmsg[256] = "ecs_init(): Requested components_count (%d) cannot be satisfied with current ecs settings. __ECS_MAX_COMPONENTS (%d) macro defined in " __FILE__;
        sprintf(errmsg, errmsg, components_count, __ECS_MAX_COMPONENTS);
        ecs_set_error(errmsg);
        return ECS_INVALID_ARGUMENT;
    }

    memset(ctx, 0, sizeof(*ctx));
    ctx->comopnents_count = components_count;

    ctx->entity_size = entity_size;

    // WARNING: temporary! then we need some prealloc amount definition
    ctx->entities = calloc(__ECS_MAX_ENTITIES, ctx->entity_size);

    return ECS_OK;
}

void ecs_cleanup(struct ecs_ctx* ctx) {
    if (ctx->entities != NULL)
        free(ctx->entities);
}
