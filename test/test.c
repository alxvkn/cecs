#include <stdint.h>
#include <sys/types.h>

#include <cecs.h>

#include <stdio.h>

// COMPONENTS

const static ecs_component_mask_t position_mask = 1;
struct position {
    int x, y;
};

const static ecs_component_mask_t velocity_mask = 1 << 1;
struct velocity {
    int x, y;
};

// SYSTEMS

ECS_DEFINE_SYSTEM(movement , position_mask | velocity_mask) {
    struct position* p = (struct position*)ecs_get_component(ctx, position_mask, entity_id);
    struct velocity* v = (struct velocity*)ecs_get_component(ctx, velocity_mask, entity_id);

    printf("movement_system called with p = {\n"
           "    x = %d\n\ty = %d\n"
           "} and v = {\n"
           "    x = %d\n\ty = %d\n"
           "}\n",
           p->x, p->y, v->x, v->y);

    p->x += v->x;
    p->y += v->y;
}

// TODO: allow systems to have zero component mask
// to be ran on every entity
ECS_DEFINE_SYSTEM(debug, position_mask) {
    printf("debug system on entity with id %lu\n", entity_id);
}

int main() {
    struct ecs_ctx ctx = {0};

    enum ecs_err err = ecs_init(&ctx,
        &(struct ecs_config) {
            .entities_pool_size = 2,
            .systems_pool_size = 2,
            .components_pool_pool_size = 2,
        });

    ecs_register_component(&ctx, position_mask, sizeof(struct position));
    ecs_register_component(&ctx, velocity_mask, sizeof(struct velocity));

    ecs_register_system(&ctx, &movement);
    ecs_register_system(&ctx, &debug);

    size_t e0 = ecs_add_entity(&ctx, position_mask | velocity_mask);
    size_t e1 = ecs_add_entity(&ctx, position_mask | velocity_mask);

    ((struct velocity*)ecs_get_component(&ctx, velocity_mask, e0))->x = 1;

    ecs_run(&ctx);
    ecs_run(&ctx);

    if (err)
        puts(ecs_get_error());
    else
        puts("looks like succesfull ecs_init() call.");

    ecs_cleanup(&ctx);

    puts("no way! finishing without segfaults!");
    return 0;
}
