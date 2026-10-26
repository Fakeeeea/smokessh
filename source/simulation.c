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

simulation init_simulation(const smoke_config config, const ivec2 size) {
    simulation s;

    s.config = config;

    init_env(&s.env, size);
    parse_emitters(s.config, &s.emitters, &s.e_num, size);
    s.obstacle = init_obstacle(size, s.config.obstacle_radius);

    reset_env(&s.env, config);

    return s;
}

void free_simulation(simulation *s) {
    free_env(&s->env);
    free_emitters(&s->emitters, &s->e_num);
}

void restart_simulation(simulation *s, ivec2 size) {
    free_env(&s->env);
    init_env(&s->env, size);
    reset_env(&s->env, s->config);
    s->obstacle = init_obstacle(size, s->config.obstacle_radius);
}

void run(simulation* sim) {
    handle_emitter(&sim->env, sim->emitters, sim->e_num);
    update_solid_map(&sim->env, sim->config, sim->obstacle);

    advect_vx(&sim->env, sim->config), advect_vy(&sim->env, sim->config);
    sim->env.vx_idx = 1 - sim->env.vx_idx, sim->env.vy_idx = 1 - sim->env.vy_idx;

    apply_temperature(&sim->env, sim->config);
    sim->env.vy_idx = 1 - sim->env.vy_idx;

    apply_wind(&sim->env, sim->config);

    solve_pressure(&sim->env, sim->config);
    update_vx(&sim->env, sim->config), update_vy(&sim->env, sim->config);
    sim->env.vx_idx = 1 - sim->env.vx_idx, sim->env.vy_idx = 1 - sim->env.vy_idx;

    advect_smoke(&sim->env, sim->config);
    sim->env.s_idx = 1 - sim->env.s_idx;
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

void reset_env(environment* env, const smoke_config config) {
    for (int y = 0; y < env->size.y; ++y) {
        for (int x = 0; x < env->size.x; ++x) {

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

    for (int y = 0; y < env->size.y; ++y) {
        for (int x = 0; x < env->size.x+1; ++x) {
            int idx = IDX(x,y,env->size.x+1);

            env->velocitiesx[0][idx] = 0;
            env->velocitiesx[1][idx] = 0;
        }
    }

    for (int y = 0; y < env->size.y+1; ++y) {
        for (int x = 0; x < env->size.x; ++x) {
            int idx = IDX(x,y,env->size.x);

            env->velocitiesy[0][idx] = 0;
            env->velocitiesy[1][idx] = 0;
        }
    }
}

void free_env(environment* env) {
    FREE(env->smoke[0]);
    FREE(env->temp[0]);
    FREE(env->smoke[1]);
    FREE(env->temp[1]);

    FREE(env->solid);
    FREE(env->pressure);

    FREE(env->velocitiesy[0]);
    FREE(env->velocitiesy[1]);
    FREE(env->velocitiesx[0]);
    FREE(env->velocitiesx[1]);
}