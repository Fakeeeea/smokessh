//
// Created by aaa on 16/10/2026.
//

#ifndef SMOKESSH_SMOKE_PHYSICS_H
#define SMOKESSH_SMOKE_PHYSICS_H

#include "smoke.h"

#define T_AMBIENT 273
#define CELL_SIZE 0.1f
#define TIMESTEP (0.0167f) // 1/60

int is_solid(ivec2 p);

float get_vx(ivec2 p);
float get_vy(ivec2 p);

float get_pressure(ivec2 p);

void solve_pressure_iteration();
void solve_pressure();
void update_vx();
void update_vy();
void advect_smoke();
void advect_vx();
void advect_vy();
void apply_temperature();
void update_solid_map();
void handle_input(ivec2 input);
void handle_emitter();

fvec2 get_interpolated_velocity(fvec2 p);

#endif //SMOKESSH_SMOKE_PHYSICS_H
