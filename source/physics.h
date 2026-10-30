//
// Created by aaa on 16/10/2026.
//

#ifndef SMOKESSH_SMOKE_PHYSICS_H
#define SMOKESSH_SMOKE_PHYSICS_H

#include "config.h"
#include "simulation.h"

#define T_AMBIENT 273
#define CELL_SIZE 0.1f
#define TIMESTEP (0.0167f) // 1/60

int is_solid(const environment* smoke, ivec2 p);
int in_bounds(const environment* smoke, ivec2 p);

float get_vx(const environment* smoke, ivec2 p);
float get_vy(const environment* smoke, ivec2 p);

float get_pressure(const environment* smoke, ivec2 p);

void solve_pressure_iteration(environment* smoke, smoke_config config);
void solve_pressure(environment* smoke, smoke_config config);
void update_vx(environment* smoke, smoke_config config);
void update_vy(environment* smoke, smoke_config config);
void advect_smoke(environment* smoke, smoke_config config);
void advect_vx(environment* smoke, smoke_config config);
void advect_vy(environment* smoke, smoke_config config);
void apply_temperature(environment* smoke, smoke_config config);
void apply_wind(environment* smoke, smoke_config config);
void update_solid_map(environment* smoke, smoke_config config, obstacle o);
void handle_emitter(environment* smoke, const emitter* emitters, int e_num);

fvec2 get_interpolated_velocity(const environment* smoke, smoke_config config, fvec2 p);

#endif //SMOKESSH_SMOKE_PHYSICS_H
