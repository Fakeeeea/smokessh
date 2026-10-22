//
// Created by aaa on 15/10/2026.
//
#include <unistd.h>
#include <stdlib.h>
#include "simulation.h"

#include <stdio.h>

#include "terminal.h"
#include "physics.h"
#include "config.h"
#include "utils.h"

#include "emitters.h"
#include "obstacles.h"

//todo config radius obs
simulation init_simulation(const smoke_config config, const ivec2 size) {
    simulation s;

    s.config = config;

    init_env(&s.env, size);
    parse_emitters(s.config, &s.emitters, &s.e_num);
    s.obstacle = init_obstacle(size, 3);

    reset_env(&s.env, config);

    return s;
}

void free_simulation(simulation *s) {
    free_env(&s->env);
    free_emitters(&s->emitters, &s->e_num);
}

//todo config radius obs
void restart_simulation(simulation *s, ivec2 size) {
    free_env(&s->env);
    init_env(&s->env, size);
    reset_env(&s->env, s->config);
    s->obstacle = init_obstacle(size, 3);
}

void run(environment* smoke, const smoke_config config, obstacle o, const emitter* e, const int e_num) {
    handle_emitter(smoke, e, e_num);
    update_solid_map(smoke, o);

    advect_vx(smoke, config), advect_vy(smoke, config);
    smoke->vx_idx = 1 - smoke->vx_idx, smoke->vy_idx = 1 - smoke->vy_idx;

    apply_temperature(smoke, config);
    smoke->vy_idx = 1 - smoke->vy_idx;

    solve_pressure(smoke, config);
    update_vx(smoke, config), update_vy(smoke, config);
    smoke->vx_idx = 1 - smoke->vx_idx, smoke->vy_idx = 1 - smoke->vy_idx;

    advect_smoke(smoke, config);
    smoke->s_idx = 1 - smoke->s_idx;
}

void init_env(environment* env, ivec2 size) {

    env->size = size;

    const int h = env->size.y;
    const int w = env->size.x;

    MALLOC(env->velocitiesx[0],(size_t)(w + 1) * h * sizeof(float));
    MALLOC(env->velocitiesx[1],(size_t)(w + 1) * h * sizeof(float));

    MALLOC(env->velocitiesy[0],(size_t)w * (h+1) * sizeof(float));
    MALLOC(env->velocitiesy[1],(size_t)w * (h+1) * sizeof(float));

    MALLOC(env->pressure,(size_t) w * h * sizeof(float));

    MALLOC(env->smoke[0],(size_t) w * h * sizeof(fvec3));
    MALLOC(env->smoke[1],(size_t) w * h * sizeof(fvec3));

    MALLOC(env->temp[0],(size_t) w * h * sizeof(float));
    MALLOC(env->temp[1],(size_t) w * h * sizeof(float));

    MALLOC(env->solid,(size_t) w * h * sizeof(int));

    env->s_idx = env->vx_idx = env->vy_idx = 0;
}

//todo x and y swap cache locality
void reset_env(environment* env, const smoke_config config) {
    for (int x = 0; x < env->size.x; ++x) {
        for (int y = 0; y < env->size.y; ++y) {

            int idx = IDX(x,y,env->size.x);

            if (x == 0 || y == 0 || x == env->size.x-1 || y == env->size.y-1) {
                env->solid[idx] = 1;
            } else {
                env->solid[idx] = 0;
            }

            env->smoke[0][idx] = (fvec3){0,0,0};
            env->smoke[1][idx] = (fvec3){0,0,0};
            env->temp[0][idx] = config.t_ambient;
            env->temp[1][idx] = config.t_ambient;

            env->pressure[idx] = 0;
        }
    }

    for (int x = 0; x < env->size.x+1; ++x) {
        for (int y = 0; y < env->size.y; ++y) {
            int idx = IDX(x,y,env->size.x+1);

            env->velocitiesx[0][idx] = 0;
            env->velocitiesx[1][idx] = 0;
        }
    }

    for (int x = 0; x < env->size.x; ++x) {
        for (int y = 0; y < env->size.y+1; ++y) {
            int idx = IDX(x,y,env->size.x);

            env->velocitiesy[0][idx] = 0;
            env->velocitiesy[1][idx] = 0;
        }
    }
}

void free_env(environment* env) {
    free(env->smoke[0]);
    free(env->temp[0]);
    free(env->smoke[1]);
    free(env->temp[1]);

    free(env->solid);
    free(env->pressure);

    free(env->velocitiesy[0]);
    free(env->velocitiesy[1]);
    free(env->velocitiesx[0]);
    free(env->velocitiesx[1]);
}