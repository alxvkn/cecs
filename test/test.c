#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <sys/types.h>

#include <cecs.h>

#include <stdio.h>
#include <unistd.h>
#include <SDL2/SDL.h>

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
           "    x = %d\n"
           "    y = %d\n"
           "} and v = {\n"
           "    x = %d\n"
           "    y = %d\n"
           "}\n",
           p->x, p->y, v->x, v->y);

    p->x += v->x;
    p->y += v->y;
}

// TODO: allow systems to have zero component mask
// src/cecs.c:45
ECS_DEFINE_SYSTEM(debug, position_mask) {
    printf("debug system on entity with id %lu\n", entity_id);
}

SDL_Renderer* sdl_renderer = NULL;

int init_sdl() {
    SDL_Window* window = NULL;

    SDL_CreateWindowAndRenderer(800, 600, SDL_WINDOW_SHOWN, &window, &sdl_renderer);

    SDL_ShowWindow(window);
    SDL_RenderClear(sdl_renderer);

    SDL_SetRenderDrawColor(sdl_renderer, 255, 255, 255, 255);

    return ECS_OK;
}

#define POINTS_TO_RENDER_COUNT_MAX 32
static SDL_Point points_to_render[POINTS_TO_RENDER_COUNT_MAX];
static int points_to_render_count = 0;
static int add_point_to_render(SDL_Point point) {
    if (points_to_render_count >= POINTS_TO_RENDER_COUNT_MAX) {
        return -1;
    }
    points_to_render[points_to_render_count] = point;
    return points_to_render_count++;
}

void render_sdl() {
    SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 255);
    SDL_RenderClear(sdl_renderer);

    SDL_SetRenderDrawColor(sdl_renderer, 255, 255, 255, 255);

    for (int i = 0; i < points_to_render_count; i++) {
        SDL_RenderDrawRect(sdl_renderer, &(SDL_Rect){
            .x = points_to_render[i].x,
            .y = points_to_render[i].y,
            .w = 10,
            .h = 10,
        });
    }
    SDL_RenderPresent(sdl_renderer);
    points_to_render_count = 0; // clear
}

ECS_DEFINE_SYSTEM(render, position_mask) {
    struct position* p = (struct position*)ecs_get_component(ctx, position_mask, entity_id);

    add_point_to_render((SDL_Point){
        .x = p->x,
        .y = p->y,
    });
}

void handle_sdl_events() {
}

int main() {
    struct ecs_ctx ctx = {0};

    enum ecs_err err = ecs_init(&ctx,
        &(struct ecs_config) {
            .entities_pool_size = 2,
            .systems_pool_size = 8,
            .components_pool_pool_size = 2,
        });

    ecs_register_component(&ctx, position_mask, sizeof(struct position));
    ecs_register_component(&ctx, velocity_mask, sizeof(struct velocity));

    ecs_register_system(&ctx, &movement);
    ecs_register_system(&ctx, &debug);

    init_sdl();

    ecs_register_system(&ctx, &render);

    size_t e0 = ecs_add_entity(&ctx, position_mask | velocity_mask);
    size_t e1 = ecs_add_entity(&ctx, position_mask | velocity_mask);

    ((struct velocity*)ecs_get_component(&ctx, velocity_mask, e0))->x = 1;
    ((struct velocity*)ecs_get_component(&ctx, velocity_mask, e1))->x = 2;

    int quit = 0;

    while (!quit) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) quit = 1;
        }
        ecs_run(&ctx);
        render_sdl();
        usleep(32 * 1000);
    }

    sleep(1);

    if (err)
        puts(ecs_get_error());
    else
        puts("looks like succesfull ecs_init() call.");

    ecs_cleanup(&ctx);

    puts("no way! finishing without segfaults!");
    return 0;
}
