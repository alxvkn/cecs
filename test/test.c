#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <stdlib.h>
#include <sys/types.h>

#include <cecs.h>

#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <SDL2/SDL.h>

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 800

#define POINTS_COUNT 100000

// COMPONENTS

ECS_DEFINE_COMPONENT(position, 1) {
    float x, y;
};

ECS_DEFINE_COMPONENT(velocity, 1 << 1) {
    float x, y;
};

ECS_DEFINE_COMPONENT(mass, velocity_mask << 1) {
    float mass;
};

// SYSTEMS

ECS_DEFINE_SYSTEM(movement , position_mask | velocity_mask) {
    struct position* p = ECS_GET_COMPONENT(position);
    struct velocity* v = ECS_GET_COMPONENT(velocity);

    // printf("movement_system called with p = {\n"
    //        "    x = %f\n"
    //        "    y = %f\n"
    //        "} and v = {\n"
    //        "    x = %f\n"
    //        "    y = %f\n"
    //        "}\n",
    //        p->x, p->y, v->x, v->y);

    p->x += v->x * delta_time;
    p->y += v->y * delta_time;
}

// TODO: allow systems to have zero component mask
// src/cecs.c:45
ECS_DEFINE_SYSTEM(debug, position_mask) {
    printf("debug system on entity with id %lu\n", entity_id);
}

ECS_DEFINE_SYSTEM(gravity, velocity_mask | mass_mask) {
    struct velocity* v = ECS_GET_COMPONENT(velocity);
    struct mass* m = ECS_GET_COMPONENT(mass);

    v->y += m->mass * delta_time;
}

SDL_Renderer* sdl_renderer = NULL;

int init_sdl() {
    SDL_Window* window = NULL;

    window = SDL_CreateWindow("hiii", 100, 100, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN);

    sdl_renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED
    );

    SDL_ShowWindow(window);
    SDL_RenderClear(sdl_renderer);

    SDL_SetRenderDrawColor(sdl_renderer, 255, 255, 255, 255);

    return ECS_OK;
}

#define POINTS_TO_RENDER_COUNT_MAX 100000
static SDL_FPoint points_to_render[POINTS_TO_RENDER_COUNT_MAX];
static int points_to_render_count = 0;
static int add_point_to_render(SDL_FPoint point) {
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

    SDL_RenderDrawPointsF(
        sdl_renderer,
        points_to_render,
        points_to_render_count
    );

    SDL_RenderPresent(sdl_renderer);
    points_to_render_count = 0; // clear
}

SDL_FPoint last_mouse_click;

ECS_DEFINE_SYSTEM(fly_to_mouse, position_mask | velocity_mask) {
    struct position* p = ECS_GET_COMPONENT(position);
    struct velocity* v = ECS_GET_COMPONENT(velocity);

    if (!(last_mouse_click.x == 0 && last_mouse_click.y == 0)) {
        v->x = (last_mouse_click.x - p->x) / 10;
        v->y = (last_mouse_click.y - p->y) / 10;
    }
}

ECS_DEFINE_SYSTEM(render, position_mask) {
    struct position* p = ECS_GET_COMPONENT(position);

    add_point_to_render((SDL_FPoint){
        .x = p->x,
        .y = p->y,
    });
}

int main() {
    struct ecs_ctx ctx = {0};

    enum ecs_err err = ecs_init(&ctx,
        &(struct ecs_config) {
            .entities_pool_size = POINTS_COUNT,
            .systems_pool_size = 8,
            .components_pool_pool_size = 3,
        });

    ecs_register_component(&ctx, position_mask, sizeof(struct position));
    ecs_register_component(&ctx, velocity_mask, sizeof(struct velocity));
    ecs_register_component(&ctx, mass_mask, sizeof(struct mass));

    ecs_register_system(&ctx, &movement);
    // ecs_register_system(&ctx, &debug);
    // ecs_register_system(&ctx, &gravity);
    ecs_register_system(&ctx, &fly_to_mouse);

    init_sdl();

    ecs_register_system(&ctx, &render);

    srand(time(0));
    for (int i = 0; i < POINTS_COUNT; i++) {
        size_t e = ecs_add_entity(&ctx, position_mask | velocity_mask | mass_mask);
        if (e == 0) {
            printf("couldn't create entity at i = %d\n", i);
            exit(1);
        }

        // ((struct velocity*)ecs_get_component(&ctx, velocity_mask, e))->y = rand() % 50 - 50;
        // ((struct velocity*)ecs_get_component(&ctx, velocity_mask, e))->x = rand() % 10 - 5;

        ((struct mass*)ecs_get_component(&ctx, mass_mask, e))->mass = 10;

        ((struct position*)ecs_get_component(&ctx, position_mask, e))->x = WINDOW_WIDTH / 2;
        ((struct position*)ecs_get_component(&ctx, position_mask, e))->y = WINDOW_HEIGHT / 2;
    }

    size_t e0 = ecs_add_entity(&ctx, position_mask | velocity_mask);
    size_t e1 = ecs_add_entity(&ctx, position_mask | velocity_mask);

    ((struct velocity*)ecs_get_component(&ctx, velocity_mask, e0))->x = 50;
    ((struct velocity*)ecs_get_component(&ctx, velocity_mask, e1))->x = 25;

    int quit = 0, paused = 0;

    while (!quit) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) quit = 1;
            else if (e.type == SDL_KEYUP) {
                if (e.key.keysym.sym == ' ') {
                    paused = !paused;
                } else if (e.key.keysym.sym == 'r') {
                    for (int i = 1; i < (POINTS_COUNT + 1); i++) {
                        double speed = (double)(rand() % 1000 - 500) / 10;
                        double x_speed = speed * ((double)(rand() % 2000 - 1000) / 1000);

                        double y_speed = sqrt((speed * speed) - (x_speed * x_speed));

                        if (speed < x_speed) {
                            y_speed = -y_speed;
                        }

                        ((struct velocity*)ecs_get_component(&ctx, velocity_mask, i))->x = x_speed;
                        ((struct velocity*)ecs_get_component(&ctx, velocity_mask, i))->y = y_speed;
                    }
                }
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                last_mouse_click.x = e.button.x;
                last_mouse_click.y = e.button.y;
            }
        }

        if (paused) {
            ctx.last_run_time = (struct timespec){0};
        }
        ecs_run(&ctx);
        render_sdl();
        // usleep((1000 * 1000) / 60);
    }

    SDL_Quit();

    if (err)
        puts(ecs_get_error());
    else
        puts("looks like succesfull ecs_init() call.");

    ecs_cleanup(&ctx);

    puts("no way! finishing without segfaults!");
    return 0;
}
