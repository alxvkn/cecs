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

void movement_system(struct ecs_ctx* ctx, size_t entity_id) {
    struct position* p = (struct position*)ecs_get_component(ctx, position_mask, entity_id);
    struct velocity* v = (struct velocity*)ecs_get_component(ctx, velocity_mask, entity_id);

    printf("movement_system called with p = {\n\tx = %d\n\ty = %d\n} and v = {\n\tx = %d\n\ty = %d\n}\n",
           p->x, p->y, v->x, v->y);
    // I FUCKING HATE MYSELF I CAN'T DO SHIT I CAN'T DO NOTHING EVEN SHE HATES ME

    p->x += v->x;
    p->y += v->y;
}

struct ecs_system movement = (struct ecs_system){
    .function = movement_system,
    .component_mask = position_mask | velocity_mask,
};

int main() {
    struct ecs_ctx ctx = {0};

    enum ecs_err err = ecs_init(&ctx,
        &(struct ecs_config) {
            .entities_pool_size = 2,
            .systems_pool_size = 1,
            .components_pool_pool_size = 2,
        });

    ecs_register_component(&ctx, position_mask, sizeof(struct position));
    ecs_register_component(&ctx, velocity_mask, sizeof(struct velocity));

    ecs_register_system(&ctx, &movement);

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
