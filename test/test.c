#include <stdint.h>
#include <sys/types.h>

#include <cecs.h>

#include <stdio.h>

// COMPONENTS

struct position {
    int x, y;
};

struct velocity {
    int x, y;
};

// THE ENTITY STRUCT

struct entity {
    component_mask_t component_mask;
    struct position pc;
    struct velocity vc;
};

// SYSTEMS

void movement_system(struct entity* e) {
    e->pc.x += e->vc.x;
    e->pc.y += e->vc.y;
}

int main() {
    struct ecs_ctx ctx = {0};

    enum ecs_err err = ecs_init(&ctx,
        &(struct ecs_config) {
            .components_count = 8,
            .entity_size = sizeof(struct entity),
        });

    if (err)
        puts(ecs_get_error());
    else
        puts("looks likes succesfull ecs_init() call.");

    ecs_cleanup(&ctx);

    puts("no way! finishing without segfaults!");
    return 0;
}
