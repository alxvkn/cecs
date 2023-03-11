#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <cecs.h>
#include <error.h>

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

enum ecs_err ecs_config_validate(const struct ecs_config* config) {
    // compare values to defined maximums
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

    return ECS_OK;
}

enum ecs_err ecs_init(struct ecs_ctx* ctx, struct ecs_config* config) {
    // invalid config parameters checking
    enum ecs_err err = ecs_config_validate(config);
    if (err) return err;

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
