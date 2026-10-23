//
// Created by aaa on 15/10/2026.
//

#ifndef SMOKESSH_SMOKE_H
#define SMOKESSH_SMOKE_H

#include "sim_math.h"
#include "config.h"
#include "obstacles.h"
#include "emitters.h"

#include <signal.h>

#define MAX_ANY_SIZE 128
#define MIN_ANY_SIZE 10

typedef struct environment {
    ivec2 size;

    float* velocitiesx[2];
    float* velocitiesy[2];

    float* pressure;
    fvec3* smoke[2];
    float* temp[2];

    int* solid;

    int vx_idx, vy_idx;
    int s_idx;
}environment;

typedef struct simulation {
    environment env;

    obstacle obstacle;
    emitter* emitters;
    int e_num;

    smoke_config config;
}simulation;

void run(simulation* sim);

simulation init_simulation(smoke_config config, ivec2 size);
void free_simulation(simulation* s);
void restart_simulation(simulation* s, ivec2 size);

void init_env(environment* env, ivec2 size);
void reset_env(environment* env, smoke_config config);
void free_env(environment* env);

#endif //SMOKESSH_SMOKE_H
