//
// Created by aaa on 15/10/2026.
//
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include "smoke.h"

#include <stdio.h>

#include "terminal.h"
#include "physics.h"
#include "graphics.h"
#include "config.h"
#include "utils.h"

environment smoke;
ivec2 size = {DEFAULT_SIZE_X , DEFAULT_SIZE_Y};
emitter* e = NULL;
int e_num = 0;

obstacle o = {
    .radius = 2,
    .location = {DEFAULT_SIZE_X * 0.5f,DEFAULT_SIZE_Y * 0.5f}
};

struct sigaction sa = {0};

void run() {
    handle_emitter();
    update_solid_map();

    advect_vx(), advect_vy();
    smoke.vx_idx = 1 - smoke.vx_idx, smoke.vy_idx = 1 - smoke.vy_idx;

    apply_temperature();
    smoke.vy_idx = 1 - smoke.vy_idx;

    solve_pressure();
    update_vx(), update_vy();
    smoke.vx_idx = 1 - smoke.vx_idx, smoke.vy_idx = 1 - smoke.vy_idx;

    advect_smoke();
    smoke.s_idx = 1 - smoke.s_idx;
}

void init_smoke() {

    smoke.size = (ivec2){size.x, size.y};

    const int h = smoke.size.y;
    const int w = smoke.size.x;

    MALLOC(smoke.velocitiesx[0],(size_t)(w + 1) * h * sizeof(float));
    MALLOC(smoke.velocitiesx[1],(size_t)(w + 1) * h * sizeof(float));

    MALLOC(smoke.velocitiesy[0],(size_t)w * (h+1) * sizeof(float));
    MALLOC(smoke.velocitiesy[1],(size_t)w * (h+1) * sizeof(float));

    MALLOC(smoke.pressure,(size_t) w * h * sizeof(float));

    MALLOC(smoke.smoke[0],(size_t) w * h * sizeof(fvec3));
    MALLOC(smoke.smoke[1],(size_t) w * h * sizeof(fvec3));

    MALLOC(smoke.temp[0],(size_t) w * h * sizeof(float));
    MALLOC(smoke.temp[1],(size_t) w * h * sizeof(float));

    MALLOC(smoke.solid,(size_t) w * h * sizeof(int));

    smoke.s_idx = smoke.vx_idx = smoke.vy_idx = 0;
}

void reset_smoke() {
    for (int x = 0; x < smoke.size.x; ++x) {
        for (int y = 0; y < smoke.size.y; ++y) {

            int idx = IDX(x,y,smoke.size.x);

            if (x == 0 || y == 0 || x == smoke.size.x-1 || y == smoke.size.y-1) {
                smoke.solid[idx] = 1;
            } else {
                smoke.solid[idx] = 0;
            }

            smoke.smoke[0][idx] = (fvec3){0,0,0};
            smoke.smoke[1][idx] = (fvec3){0,0,0};
            smoke.temp[0][idx] = config.t_ambient;
            smoke.temp[1][idx] = config.t_ambient;

            smoke.pressure[idx] = 0;
        }
    }

    for (int x = 0; x < smoke.size.x+1; ++x) {
        for (int y = 0; y < smoke.size.y; ++y) {
            int idx = IDX(x,y,smoke.size.x+1);

            smoke.velocitiesx[0][idx] = 0;
            smoke.velocitiesx[1][idx] = 0;
        }
    }

    for (int x = 0; x < smoke.size.x; ++x) {
        for (int y = 0; y < smoke.size.y+1; ++y) {
            int idx = IDX(x,y,smoke.size.x);

            smoke.velocitiesy[0][idx] = 0;
            smoke.velocitiesy[1][idx] = 0;
        }
    }
}

void free_smoke() {
    free(smoke.smoke[0]);
    free(smoke.temp[0]);
    free(smoke.smoke[1]);
    free(smoke.temp[1]);

    free(smoke.solid);
    free(smoke.pressure);

    free(smoke.velocitiesy[0]);
    free(smoke.velocitiesy[1]);
    free(smoke.velocitiesx[0]);
    free(smoke.velocitiesx[1]);
}

void init_obstacle() {
    o.location.x = (int)((float)smoke.size.x * 0.5f);
    o.location.y = (int)((float)smoke.size.y * 0.5f);
}

void restart_simulation() {
    free_smoke();
    detect_size(&size);
    init_smoke();
    reset_smoke();
    init_graphics();
    init_obstacle();
}

void free_emitters() {
    if (e != NULL) {
        free(e);
        e = NULL;
        e_num = 0;
    }
}

void set_handlers() {
    sa.sa_handler = resize_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    sigaction(SIGWINCH, &sa, NULL);
}

long get_elapsed_us(struct timespec start_time, struct timespec end_time) {
    return (end_time.tv_sec - start_time.tv_sec) * 1000000L + (end_time.tv_nsec - start_time.tv_nsec) / 1000L;
}