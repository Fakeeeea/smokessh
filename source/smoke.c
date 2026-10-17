//
// Created by aaa on 15/10/2026.
//
#include <unistd.h>
#include <time.h>

#include <stdlib.h>
#include "smoke.h"

#include "terminal.h"
#include "smoke_physics.h"
#include "smoke_graphics.h"
#include "smoke_parser.h"

environment smoke;

ivec2 size = {100, 30};

obstacle o = {
    .radius = 3,
    .location = {30,15}
};

emitter e1 = {
    .location = {30, 3},
    .densities = {5, 10, 10},
    .temperature = 300,
};

emitter e2 = {
    .location = {70, 3},
    .densities = {10, 10, 5},
    .temperature = 300,
};

emitter* e = NULL;
int e_num = 0;

void run() {
    handle_emitter();
    update_solid_map();

    advect_vx(), advect_vy();
    smoke.vx_idx = 1 - smoke.vx_idx, smoke.vy_idx = 1 - smoke.vy_idx;

    apply_temperature();
    smoke.vy_idx = 1 - smoke.vy_idx;

    for (int x = 0; x < smoke.size.x; ++x) {
        for (int y = 0; y < smoke.size.y; ++y) {
            smoke.pressure[x][y] = 0;
        }
    }

    solve_pressure();
    update_vx(), update_vy();
    smoke.vx_idx = 1 - smoke.vx_idx, smoke.vy_idx = 1 - smoke.vy_idx;

    advect_smoke();
    smoke.s_idx = 1 - smoke.s_idx;
}

void init_smoke() {
    smoke.size = (ivec2){size.x, size.y};

    smoke.velocitiesx[0] = malloc((smoke.size.x + 1) * sizeof(float*));
    smoke.velocitiesx[1] = malloc((smoke.size.x + 1) * sizeof(float*));

    smoke.velocitiesy[0] = malloc(smoke.size.x * sizeof(float*));
    smoke.velocitiesy[1] = malloc(smoke.size.x * sizeof(float*));

    smoke.pressure = malloc(smoke.size.x * sizeof(float*));

    smoke.smoke[0] = malloc(smoke.size.x * sizeof(fvec3*));
    smoke.smoke[1] = malloc(smoke.size.x * sizeof(fvec3*));

    smoke.temp[0] = malloc(smoke.size.x * sizeof(float*));
    smoke.temp[1] = malloc(smoke.size.x * sizeof(float*));

    smoke.solid = malloc(smoke.size.x * sizeof(int*));

    for (int i = 0; i <= smoke.size.x; i++) {
        smoke.velocitiesx[0][i] = malloc(smoke.size.y * sizeof(float));
        smoke.velocitiesx[1][i] = malloc(smoke.size.y * sizeof(float));

        smoke.velocitiesy[0][i] = malloc((smoke.size.y + 1) * sizeof(float));
        smoke.velocitiesy[1][i] = malloc((smoke.size.y + 1) * sizeof(float));

        smoke.pressure[i] = malloc(smoke.size.y * sizeof(float));

        smoke.smoke[0][i] = malloc(smoke.size.y * sizeof(fvec3));
        smoke.smoke[1][i] = malloc(smoke.size.y * sizeof(fvec3));

        smoke.temp[0][i] = malloc(smoke.size.y * sizeof(float));
        smoke.temp[1][i] = malloc(smoke.size.y * sizeof(float));

        smoke.solid[i] = malloc(smoke.size.y * sizeof(int));
    }

    smoke.s_idx = smoke.vx_idx = smoke.vy_idx = 0;
}

void reset_smoke() {
    for (int x = 0; x < smoke.size.x; ++x) {
        for (int y = 0; y < smoke.size.y; ++y) {
            if (x == 0 || y == 0 || x == smoke.size.x-1 || y == smoke.size.y-1) {
                smoke.solid[x][y] = 1;
            } else {
                smoke.solid[x][y] = 0;
            }

            smoke.smoke[0][x][y] = (fvec3){0,0,0};
            smoke.smoke[1][x][y] = (fvec3){0,0,0};
            smoke.temp[0][x][y] = T_AMBIENT;
            smoke.temp[1][x][y] = T_AMBIENT;

            smoke.pressure[x][y] = 0;
        }
    }

    for (int x = 0; x < smoke.size.x+1; ++x) {
        for (int y = 0; y < smoke.size.y; ++y) {
            smoke.velocitiesx[0][x][y] = 0;
            smoke.velocitiesx[1][x][y] = 0;
        }
    }

    for (int x = 0; x < smoke.size.x; ++x) {
        for (int y = 0; y < smoke.size.y+1; ++y) {
            smoke.velocitiesy[0][x][y] = 0;
            smoke.velocitiesy[1][x][y] = 0;
        }
    }
}

void free_smoke() {
    for (int x = 0; x < smoke.size.x; ++x) {
        free(smoke.solid[x]);
        free(smoke.pressure[x]);

        free(smoke.smoke[0][x]);
        free(smoke.smoke[1][x]);
        free(smoke.temp[0][x]);
        free(smoke.temp[1][x]);

        free(smoke.velocitiesy[0][x]);
        free(smoke.velocitiesy[1][x]);
    }
    free(smoke.smoke[0]);
    free(smoke.temp[0]);
    free(smoke.smoke[1]);
    free(smoke.temp[1]);

    free(smoke.solid);
    free(smoke.pressure);

    free(smoke.velocitiesy[0]);
    free(smoke.velocitiesy[1]);

    for (int x = 0; x < smoke.size.x+1; ++x) {
            free(smoke.velocitiesx[0][x]);
            free(smoke.velocitiesx[1][x]);
    }
    free(smoke.velocitiesx[0]);
    free(smoke.velocitiesx[1]);
}

int main() {
    init_smoke();
    reset_smoke();

    parse_emitters(&e, &e_num);
    setup_terminal();

    const float TARGET_FPS = 15.0f;
    const long target_frame_time_us = (long)(1000000.0f / TARGET_FPS);

    struct timespec start_time, end_time;

    while (1) {
        clock_gettime(CLOCK_MONOTONIC, &start_time);

        ivec2 mov = parse_input();

        if (mov.x == 42 && mov.y == 42) {
            break;
        }

        handle_input(mov);
        run();

        print_smoke();

        clock_gettime(CLOCK_MONOTONIC, &end_time);

        long elapsed_us = (end_time.tv_sec - start_time.tv_sec) * 1000000L + (end_time.tv_nsec - start_time.tv_nsec) / 1000L;
        if (elapsed_us < target_frame_time_us) {
            usleep(target_frame_time_us - elapsed_us);
        }
    }

    reset_terminal();
    return 0;
}