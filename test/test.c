#include <SDL3/SDL_blendmode.h>
#include <SDL3/SDL_camera.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_video.h>
#include <stdlib.h>
#include <sys/types.h>

#include <cecs.h>

#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define POINTS_COUNT 100000

// COMPONENTS

ECS_DEFINE_COMPONENT(position, 1) {
    float x, y;
};

ECS_DEFINE_COMPONENT(velocity, 1 << 1) {
    float x, y;
};

ECS_DEFINE_COMPONENT(acceleration, velocity_mask << 1) {
    float x, y;
};

ECS_DEFINE_COMPONENT(mass, acceleration_mask << 1) {
    float mass;
};

// SYSTEMS

ECS_DEFINE_SYSTEM(velocity_update, velocity_mask | acceleration_mask) {
    struct velocity* v = ECS_GET_COMPONENT(velocity);
    struct acceleration* a = ECS_GET_COMPONENT(acceleration);

    struct velocity new_v = {0};

    v->x += a->x * delta_time;
    v->y += a->y * delta_time;
}

ECS_DEFINE_SYSTEM(movement, position_mask | velocity_mask) {
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

    struct position new_p = {0};

    new_p.x = p->x + v->x * delta_time;
    new_p.y = p->y + v->y * delta_time;

    // if (new_p.x > WINDOW_WIDTH || new_p.x < 0) {
    //     v->x = v->x / -5;
    //     new_p.x = p->x + v->x * delta_time;
    // }
    //
    // if (new_p.y > WINDOW_HEIGHT || new_p.y < 0) {
    //     v->y = v->y / -5;
    //     new_p.y = p->y + v->y * delta_time;
    // }

    *p = new_p;
}

ECS_DEFINE_SYSTEM(bounce_from_bounds, position_mask | velocity_mask) {
    struct position* p = ECS_GET_COMPONENT(position);
    struct velocity* v = ECS_GET_COMPONENT(velocity);

    if (p->x > WINDOW_WIDTH || p->x < 0) {
        v->x = -v->x;
    }

    if (p->y > WINDOW_HEIGHT || p->y < 0) {
        v->y = -v->y;
    }
}

// TODO: allow systems to have zero component mask
// src/cecs.c:45
ECS_DEFINE_SYSTEM(debug, position_mask) {
    printf("debug system on entity with id %lu\n", entity_id);
}

static int run_gravity = 1;

ECS_DEFINE_SYSTEM(gravity, velocity_mask | mass_mask) {
    const float g = 9.8;
    struct acceleration* a = ECS_GET_COMPONENT(acceleration);
    struct mass* m = ECS_GET_COMPONENT(mass);

    if (run_gravity) a->y += g * m->mass * delta_time;
}

SDL_Renderer* sdl_renderer = NULL;
SDL_Camera* sdl_camera = NULL;
TTF_Font* sdl_font = NULL;

int init_sdl() {
    SDL_Window* window = NULL;

    window = SDL_CreateWindow(
        "hiii",
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        0
    );

    sdl_renderer = SDL_CreateRenderer(
        window,
        NULL
    );

    SDL_Init(SDL_INIT_CAMERA);

    SDL_CameraID* devices = NULL;
    int devices_count = 0;
    devices = SDL_GetCameras(&devices_count);

    if (devices_count == 0) {
        SDL_Log("couldn't find cameras: %s", SDL_GetError());
        SDL_Quit();
    }

    sdl_camera = SDL_OpenCamera(devices[0], NULL);
    if (!sdl_camera) {
        SDL_Log("couldn't open camera: %s", SDL_GetError());
        SDL_Quit();
    }

    SDL_free(devices);

    TTF_Init();

    sdl_font = TTF_OpenFont("/usr/share/fonts/liberation/LiberationSerif-Regular.ttf", 50);
    // TTF_Font* emojis = TTF_OpenFont("/usr/share/fonts/joypixels/JoyPixels.ttf", 50);
    // TTF_AddFallbackFont(sdl_font, emojis);

    SDL_ShowWindow(window);
    SDL_RenderClear(sdl_renderer);

    SDL_SetRenderDrawColor(sdl_renderer, 255, 255, 255, 255);

    return ECS_OK;
}

#define POINTS_TO_RENDER_COUNT_MAX 50000
static SDL_FPoint points_to_render[POINTS_TO_RENDER_COUNT_MAX];
static int points_to_render_count = 0;
static int add_point_to_render(SDL_FPoint point) {
    if (points_to_render_count >= POINTS_TO_RENDER_COUNT_MAX) {
        return -1;
    }
    points_to_render[points_to_render_count] = point;
    return points_to_render_count++;
}

void render_text_sdl(TTF_Font* font, const char* text, size_t length) {
    const static SDL_Color white = { 255, 255, 255 };
    const static SDL_Color grey = { 127, 127, 127 };
    SDL_Surface* surface = TTF_RenderText_Shaded(font, text, length, white, grey);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(sdl_renderer, surface);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_ADD);

    SDL_FRect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = 300;
    rect.h = 50;

    SDL_RenderTexture(sdl_renderer, texture, NULL, &rect);

    SDL_DestroySurface(surface);
    SDL_DestroyTexture(texture);
}

void render_points_sdl() {
    SDL_SetRenderDrawColor(sdl_renderer, 255, 255, 255, 255);

    SDL_RenderPoints(
        sdl_renderer,
        points_to_render,
        points_to_render_count
    );

    points_to_render_count = 0; // clear
}

void render_camera_sdl() {
    static SDL_Texture* texture = NULL;

    Uint64 timestamp = 0;
    SDL_Surface* frame = SDL_AcquireCameraFrame(sdl_camera, &timestamp);

    if (frame != NULL) {
        if (!texture) {
            texture = SDL_CreateTexture(sdl_renderer, frame->format, SDL_TEXTUREACCESS_STREAMING, frame->w, frame->h);
        }

        if (texture) {
            SDL_UpdateTexture(texture, NULL, frame->pixels, frame->pitch);
        }
    }

    SDL_ReleaseCameraFrame(sdl_camera, frame);

    if (texture) {
        SDL_RenderTexture(sdl_renderer, texture, NULL, NULL);
    }
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

void randomize_velocities(struct ecs_ctx* ctx) {
    for (int i = 1; i < (POINTS_COUNT + 1); i++) {
        double speed = (double)(rand() % 1000 - 500) / 10;
        double x_speed = speed * ((double)(rand() % 2000 - 1000) / 1000);

        double y_speed = sqrt((speed * speed) - (x_speed * x_speed));

        if (speed < x_speed) {
            y_speed = -y_speed;
        }

        ((struct velocity*)ecs_get_component(ctx, acceleration_mask, i))->x = 0;
        ((struct velocity*)ecs_get_component(ctx, acceleration_mask, i))->y = 0;
        ((struct velocity*)ecs_get_component(ctx, velocity_mask, i))->x = x_speed;
        ((struct velocity*)ecs_get_component(ctx, velocity_mask, i))->y = y_speed;
    }
}


int main() {
    struct ecs_ctx ctx = {0};

    enum ecs_err err = ecs_init(&ctx,
                                &(struct ecs_config) {
                                .entities_pool_size = POINTS_COUNT,
                                .systems_pool_size = 8,
                                .components_pool_pool_size = 4,
                                });

    ecs_register_component(&ctx, position_mask, sizeof(struct position));
    ecs_register_component(&ctx, velocity_mask, sizeof(struct velocity));
    ecs_register_component(&ctx, acceleration_mask, sizeof(struct acceleration));
    ecs_register_component(&ctx, mass_mask, sizeof(struct mass));

    ecs_register_system(&ctx, &velocity_update);
    ecs_register_system(&ctx, &movement);
    ecs_register_system(&ctx, &bounce_from_bounds);
    ecs_register_system(&ctx, &gravity);
    // ecs_register_system(&ctx, &debug);
    // ecs_register_system(&ctx, &fly_to_mouse);

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
    randomize_velocities(&ctx);

    size_t e0 = ecs_add_entity(&ctx, position_mask | velocity_mask);
    size_t e1 = ecs_add_entity(&ctx, position_mask | velocity_mask);

    ((struct velocity*)ecs_get_component(&ctx, velocity_mask, e0))->x = 50;
    ((struct velocity*)ecs_get_component(&ctx, velocity_mask, e1))->x = 25;

    int quit = 0, paused = 0;

    while (!quit) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) quit = 1;
            else if (e.type == SDL_EVENT_KEY_UP) {
                if (e.key.key == ' ') {
                    paused = !paused;
                } else if (e.key.key == 'r') {
                    randomize_velocities(&ctx);
                } else if (e.key.key == 'g') {
                    run_gravity = !run_gravity;
                } else if (e.key.key == 'q') {
                    quit = 1;
                }
            } else if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                last_mouse_click.x = e.button.x;
                last_mouse_click.y = e.button.y;
            }
        }

        if (paused) {
            ctx.last_run_time = (struct timespec){0};
        }
        double delta_time = ecs_run(&ctx);
        const static size_t delta_time_string_capacity = 128;
        char delta_time_string[delta_time_string_capacity];
        const size_t delta_time_string_size = snprintf(delta_time_string, delta_time_string_capacity, "%f Δt", delta_time);

        SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 255);
        SDL_RenderClear(sdl_renderer);

        render_camera_sdl();
        render_points_sdl();
        render_text_sdl(sdl_font, delta_time_string, delta_time_string_size);

        SDL_RenderPresent(sdl_renderer);
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
