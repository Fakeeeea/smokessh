//
// Created by aaa on 15/10/2026.
//

#ifndef SMOKESSH_SMOKE_H
#define SMOKESSH_SMOKE_H

#include "smoke_math.h"

#include <signal.h>

#define MAX_ANY_SIZE 128
#define MIN_ANY_SIZE 10

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

    volatile sig_atomic_t resize_pending;
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

extern environment smoke;
extern ivec2 size;
extern emitter* e;
extern int e_num;

void run();

void init_smoke();
void reset_smoke();
void free_smoke();
void init_obstacle();
void restart_simulation();
void set_handlers();
long get_elapsed_us(struct timespec start_time, struct timespec end_time);
void free_emitters();

#endif //SMOKESSH_SMOKE_H
