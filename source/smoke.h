//
// Created by aaa on 15/10/2026.
//

#ifndef SMOKESSH_SMOKE_H
#define SMOKESSH_SMOKE_H

#include "smoke_math.h"

typedef struct environment {
    ivec2 size;

    float** velocitiesx[2];
    float** velocitiesy[2];

    float** pressure;
    fvec3** smoke[2];
    float** temp[2];

    int** solid;

    int vx_idx, vy_idx;
    int s_idx;
}environment;

typedef struct emitter {
    ivec2 location;
    fvec3 densities;
    float temperature;
}emitter;

typedef struct obstacle {
    ivec2 location;
    int radius;
}obstacle;

void run();

void init_smoke();
void reset_smoke();
void free_smoke();

void handle_emitter();

fvec2 get_interpolated_velocity(fvec2 p);

int print_smoke();

#endif //SMOKESSH_SMOKE_H
