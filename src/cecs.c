#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <cecs.h>

#define __ECS_ERROR_MSG_BUF_SIZE 256

// later something like (sizeof(component_mask_t) * CHAR_BIT)
#define __ECS_MAX_COMPONENTS 8

#define __ECS_MAX_ENTITIES 8

// user aren't supposed to modify ctx structure directly
// but only use documented functions
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
static inline void _ecs_set_error(const char* file_and_line, const char* func, const char* message) {
    ecs_set_error(file_and_line);
    strcat(error_msg_buf, func);
    strcat(error_msg_buf, message);
}

#define __ECS_SET_ERROR_HELPER(file, line, func, error) _ecs_set_error("CECS error: "file ":" #line " ", func, "(): " error)
#define __ECS_SET_ERROR_LINE_HELPER(file, line, func, error) __ECS_SET_ERROR_HELPER(file, line, func, error)
#define ECS_SET_ERROR(ERROR) __ECS_SET_ERROR_LINE_HELPER(__FILE__, __LINE__, __func__, ERROR)
// i think this system with macros and buffer for errors
// is not flexible and any good at all, but at this i don't know
// what else to do so i'll stay with it

const char* ecs_get_error() {
    return error_msg_buf;
}

// not static because after code splitting into different files we should be able to
// extern struct ecs_ctx* current_ctx;
// which (i guess) will be more optimal then calling ecs_get_ctx()
// ^^^^ that's not to think about threads :(
struct ecs_ctx* current_ctx = {0};

void ecs_set_ctx(struct ecs_ctx* ctx) {
    current_ctx = ctx;
}

struct ecs_ctx* ecs_get_ctx() {
    return current_ctx;
}

enum ecs_err ecs_init(struct ecs_ctx* ctx, struct ecs_config* config) {
    // invalid config parameters checking
    if (config->entity_size == 0) {
        ECS_SET_ERROR("entity_size cannot be 0");
        return ECS_INVALID_ARGUMENT;
    }
    if (config->components_count < 1) {
        ECS_SET_ERROR("There must be at least one component!");
        return ECS_INVALID_ARGUMENT;
    }
    if (config->components_count > __ECS_MAX_COMPONENTS) {
        char errmsg[__ECS_ERROR_MSG_BUF_SIZE] = "ecs_init(): Requested components_count (%d) cannot be satisfied with current ecs settings. __ECS_MAX_COMPONENTS (%d) macro defined in " __FILE__;
        sprintf(errmsg, errmsg, config->components_count, __ECS_MAX_COMPONENTS);
        ecs_set_error(errmsg);
        return ECS_INVALID_ARGUMENT;
    }

    // all parameters are fine, actual initialization
    memset(ctx, 0, sizeof(*ctx));
    ctx->components_count = config->components_count;

    ctx->entities.size = config->entity_size;

    // TODO: temporary! then we need some prealloc amount definition
    ctx->entities.array = calloc(__ECS_MAX_ENTITIES, config->entity_size);

    return ECS_OK;
}

void ecs_cleanup(struct ecs_ctx* ctx) {
    if (ctx->entities.array != NULL)
        free(ctx->entities.array);
}
