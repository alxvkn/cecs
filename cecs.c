#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define __ECS_ERROR_MSG_BUF_SIZE 128

#define __ECS_MAX_COMPONENTS 8
#define __ECS_MAX_ENTITIES 8

struct ecs_ctx {
    size_t entity_size;
    size_t* components_sizes;

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

// i think we should not use ctx structure directly
// but provide functions instead
//
// in this realization we won't be able to dynamicly
// register new components
//
// maybe ecs functions/ecs ctx doesn't need to know about the components?
// ^^^ upper idea is like uhm so aaa mm not good?
    // since we need to query entities
    // which have components of some type by ecs function

// we should maybe make this just a pointer later,
// since error messages propably will only be string literals
static char error_msg_buf[__ECS_ERROR_MSG_BUF_SIZE];
static void ecs_set_error(const char* message) {
    strcpy(error_msg_buf, message);
}

const char* ecs_get_error() {
    return error_msg_buf;
}

enum ecs_err ecs_init(struct ecs_ctx* ctx, unsigned int components_count, size_t component0_size, ...) {
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

    ctx->entity_size = 0;

    ctx->components_sizes = malloc(sizeof(size_t) * components_count);
    if (ctx->components_sizes == NULL) {
        ecs_set_error("ecs_init(): Failed to allocate memory for array of component sizes.");
        return ECS_ALLOC_FAILURE;
    }
    va_list sizes;
    va_start(sizes, component0_size);
    for (unsigned i = 0; i < components_count; i++) {
        ctx->components_sizes[i] = va_arg(sizes, size_t);
        ctx->entity_size += ctx->components_sizes[i];
    }
    va_end(sizes);

    ctx->entities = malloc(ctx->entity_size * __ECS_MAX_ENTITIES); // WARNING: temporary! then we need some prealloc amount definition

    return ECS_OK;
}

void ecs_cleanup(struct ecs_ctx* ctx) {
    if (ctx->components_sizes != NULL)
        free(ctx->components_sizes);

    if (ctx->entities != NULL)
        free(ctx->entities);
}

int main() {
    struct ecs_ctx ctx;
    enum ecs_err err = ecs_init(&ctx, 8, 0);
    if (err)
        puts(ecs_get_error());
    else
        puts("looks likes succesfull ecs_init() call.");

    ecs_cleanup(&ctx);

    puts("no way! finishing without segfaults!");
    return 0;
}
