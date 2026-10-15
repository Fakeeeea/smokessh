//
// Created by aaa on 15/10/2026.
//
#include <sys/ioctl.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include "smoke.h"

#include "terminal.h"

#define T_AMBIENT 273

struct winsize terminal_size;

environment smoke;

obstacle player;
emitter e;

void run() {
    //handle_emitter
    //handle player

    advect_vx(), advect_vy();
    smoke.vx_idx = 1 - smoke.vx_idx, smoke.vy_idx = 1 - smoke.vy_idx;

    apply_temperature();
    smoke.vy_idx = 1 - smoke.vy_idx;

    solve_pressure();
    update_vx(), update_vy();
    smoke.vx_idx = 1 - smoke.vx_idx, smoke.vy_idx = 1 - smoke.vy_idx;

    advect_smoke();
    smoke.s_idx = 1 - smoke.s_idx;
    print_smoke();
}

void init_smoke() {
    smoke.size = (ivec2){terminal_size.ws_row, terminal_size.ws_col};

    smoke.velocitiesx[0] = malloc(smoke.size.x * sizeof(float*) + 1);
    for (int i = 0; i < smoke.size.x; i++) {
        smoke.velocitiesx[0][i] = malloc(smoke.size.y * sizeof(float));
    }

    smoke.velocitiesx[1] = malloc(smoke.size.x * sizeof(float*) + 1);
    for (int i = 0; i < smoke.size.x; i++) {
        smoke.velocitiesx[1][i] = malloc(smoke.size.y * sizeof(float));
    }

    smoke.velocitiesy[0] = malloc(smoke.size.x * sizeof(float*));
    for (int i = 0; i < smoke.size.x; i++) {
        smoke.velocitiesy[0][i] = malloc(smoke.size.y * sizeof(float) + 1);
    }

    smoke.velocitiesy[1] = malloc(smoke.size.x * sizeof(float*));
    for (int i = 0; i < smoke.size.x; i++) {
        smoke.velocitiesy[1][i] = malloc(smoke.size.y * sizeof(float) + 1);
    }

    smoke.pressure = malloc(smoke.size.x * sizeof(float*));
    for (int i = 0; i < smoke.size.x; i++) {
        smoke.pressure[i] = malloc(smoke.size.y * sizeof(float));
    }

    smoke.smoke[0] = malloc(smoke.size.x * sizeof(fvec2*));
    for (int i = 0; i < smoke.size.x; i++) {
        smoke.smoke[0][i] = malloc(smoke.size.y * sizeof(fvec2));
    }

    smoke.smoke[1] = malloc(smoke.size.x * sizeof(int*));
    for (int i = 0; i < smoke.size.x; i++) {
        smoke.smoke[1][i] = malloc(smoke.size.y * sizeof(int));
    }

    smoke.solid = malloc(smoke.size.x * sizeof(int*));
    for (int i = 0; i < smoke.size.x; i++) {
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

            smoke.smoke[0][x][y] = (fvec2){0,T_AMBIENT};
            smoke.smoke[1][x][y] = (fvec2){0,T_AMBIENT};

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

        free(smoke.velocitiesy[0][x]);
        free(smoke.velocitiesy[1][x]);
    }
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

int is_solid(ivec2 p) {
    if (p.x < 0 || p.y < 0 || p.x >= smoke.size.x || p.y >= smoke.size.y) {
        return 1;
    }

    return smoke.solid[p.x][p.y];
}

int get_vx(ivec2 p) {
    if (p.x < 0 || p.y < 0 || p.x >= (smoke.size.x + 1) || p.y >= smoke.size.y) {
        return 0;
    }

    return smoke.velocitiesx[smoke.vx_idx][p.x][p.y];
}

int get_vy(ivec2 p) {
    if (p.x < 0 || p.y < 0 || p.x >= smoke.size.x || p.y >= (smoke.size.y + 1)) {
        return 0;
    }

    return smoke.velocitiesy[smoke.vy_idx][p.x][p.y];
}

int get_pressure(ivec2 p) {
    if (p.x < 0 || p.y < 0 || p.x >= smoke.size.x || p.y >= smoke.size.y) {
        return 0;
    }

    return smoke.pressure[p.x][p.y];
}

int print_smoke() {
    const char levels[] = " .:-=+*#%@";
    int n_levels = sizeof(levels) -1;

    printf("\x1b[H");
    for (int y = smoke.size.y - 1; y >= 0; y--) {
        for (int x = 0; x < smoke.size.x; ++x) {

            if (is_solid((ivec2){x,y})) {putchar('&'); continue;}

            int level_idx = (int)(smoke.smoke[smoke.s_idx][x][y].x);
            if (level_idx < 0) level_idx = 0;
            if (level_idx >= n_levels) level_idx = n_levels -1;

            putchar(levels[level_idx]);
        }
        putchar('\n');
    }
}

int main() {
    get_terminal_size(&terminal_size);
    init_smoke();
    reset_smoke();

    while (1)
        run();
    return 0;
}