//
// Created by aaa on 16/10/2026.
//

#include "smoke_physics.h"

extern environment smoke;
extern emitter e1;
extern emitter e2;
extern obstacle o;

int is_solid(ivec2 p) {
    if (p.x < 0 || p.y < 0 || p.x >= smoke.size.x || p.y >= smoke.size.y) {
        return 1;
    }

    return smoke.solid[p.x][p.y];
}

float get_vx(ivec2 p) {
    if (p.x < 0 || p.y < 0 || p.x >= (smoke.size.x + 1) || p.y >= smoke.size.y) {
        return 0;
    }

    return smoke.velocitiesx[smoke.vx_idx][p.x][p.y];
}

float get_vy(ivec2 p) {
    if (p.x < 0 || p.y < 0 || p.x >= smoke.size.x || p.y >= (smoke.size.y + 1)) {
        return 0;
    }

    return smoke.velocitiesy[smoke.vy_idx][p.x][p.y];
}

float get_pressure(ivec2 p) {
    if (p.x < 0 || p.y < 0 || p.x >= smoke.size.x || p.y >= smoke.size.y) {
        return 0;
    }

    return smoke.pressure[p.x][p.y];
}

void solve_pressure_iteration() {
    for (int x = 0; x < smoke.size.x; ++x) {
        for (int y = 0; y < smoke.size.y; ++y) {
            int solid_left = is_solid((ivec2){x-1, y});
            int solid_right = is_solid((ivec2){x+1, y});
            int solid_top = is_solid((ivec2){x, y+1});
            int solid_bottom = is_solid((ivec2){x, y-1});

            int flow_left = 1 - solid_left;
            int flow_right = 1 - solid_right;
            int flow_top = 1 - solid_top;
            int flow_bottom = 1 - solid_bottom;

            int f_sum = flow_left + flow_right + flow_top + flow_bottom;
            if (f_sum == 0) continue;

            float v_left = get_vx((ivec2){x,y});
            float v_right = get_vx((ivec2){x+1,y});
            float v_bottom = get_vy((ivec2){x,y});
            float v_top = get_vy((ivec2){x,y+1});

            float delta_v = v_right - v_left + v_top - v_bottom;

            float p_left = get_pressure((ivec2){x-1,y}) * flow_left;
            float p_right = get_pressure((ivec2){x+1,y}) * flow_right;
            float p_top = get_pressure((ivec2){x,y+1}) * flow_top;
            float p_bottom = get_pressure((ivec2){x,y-1}) * flow_bottom;

            float p_sum = p_left + p_right + p_top + p_bottom;
            float p = (p_sum - CELL_SIZE * delta_v / TIMESTEP) / f_sum;

            smoke.pressure[x][y] = p;
        }
    }
}

void solve_pressure() {
    const float ITERATIONS = 60;
    for (int i = 0; i <= ITERATIONS; ++i) {
        solve_pressure_iteration();
    }
}
void update_vx() {
    const float k = - TIMESTEP / CELL_SIZE;
    for (int x = 0; x <= smoke.size.x; ++x) {
        for (int y = 0; y < smoke.size.y; ++y) {
            if (is_solid((ivec2){x,y}) || is_solid((ivec2){x-1,y})) continue;
            float p_right = get_pressure((ivec2){x,y});
            float p_left = get_pressure((ivec2){x-1,y});

            smoke.velocitiesx[1-smoke.vx_idx][x][y] = smoke.velocitiesx[smoke.vx_idx][x][y] + k * (p_right - p_left);
        }
    }
}
void update_vy() {
    const float k = - TIMESTEP / CELL_SIZE;
    for (int x = 0; x < smoke.size.x; ++x) {
        for (int y = 0; y <= smoke.size.y; ++y) {
            if (is_solid((ivec2){x,y}) || is_solid((ivec2){x,y-1})) continue;
            float p_top = get_pressure((ivec2){x,y});
            float p_bottom = get_pressure((ivec2){x,y-1});

            smoke.velocitiesy[1-smoke.vy_idx][x][y] = smoke.velocitiesy[smoke.vy_idx][x][y] + k * (p_top - p_bottom);
        }
    }
}

void advect_smoke() {
    for (int x = 0; x < smoke.size.x; ++x) {
        for (int y = 0; y < smoke.size.y; ++y) {
            if (is_solid((ivec2){x,y})) {
                smoke.smoke[1-smoke.s_idx][x][y] = (fvec3){0,0,0};
                smoke.temp[1-smoke.s_idx][x][y] = T_AMBIENT;
                continue;
            };

            fvec2 world_pos = {(x + 0.5f) * CELL_SIZE, (y + 0.5f) * CELL_SIZE};
            fvec2 v = get_interpolated_velocity(world_pos);
            fvec2 comes_from = {world_pos.x - v.x * TIMESTEP, world_pos.y - v.y * TIMESTEP};

            fvec2 grid_space = {comes_from.x / CELL_SIZE - 0.5f, comes_from.y / CELL_SIZE - 0.5f};
            fvec2 prev = {grid_space.x, grid_space.y};
            smoke.smoke[1-smoke.s_idx][x][y] = sample_bilinear_fvec3(&smoke.smoke[smoke.s_idx][0], smoke.size.x, smoke.size.y, prev.x, prev.y);
            smoke.temp[1-smoke.s_idx][x][y] = sample_bilinear(&smoke.temp[smoke.s_idx][0], smoke.size.x, smoke.size.y, prev.x, prev.y);;
        }
    }
}

void advect_vx() {
    for (int x = 0; x <= smoke.size.x; ++x) {
        for (int y = 0; y < smoke.size.y; ++y) {
            if (is_solid((ivec2){x,y}) || is_solid((ivec2){x-1,y})) continue;

            fvec2 world_pos = {x * CELL_SIZE, (y + 0.5f) * CELL_SIZE};
            fvec2 v = get_interpolated_velocity(world_pos);
            fvec2 comes_from = {world_pos.x - v.x * TIMESTEP, world_pos.y - v.y * TIMESTEP};

            fvec2 vx_uv = {comes_from.x / CELL_SIZE, comes_from.y / CELL_SIZE - 0.5f};
            smoke.velocitiesx[1-smoke.vx_idx][x][y] = sample_bilinear(&smoke.velocitiesx[smoke.vx_idx][0], smoke.size.x+1, smoke.size.y, vx_uv.x, vx_uv.y);;
        }
    }
}
void advect_vy() {
    for (int x = 0; x < smoke.size.x; ++x) {
        for (int y = 0; y <= smoke.size.y; ++y) {
            if (is_solid((ivec2){x,y}) || is_solid((ivec2){x,y-1})) continue;

            fvec2 world_pos = {(x + 0.5f) * CELL_SIZE, y * CELL_SIZE};
            fvec2 v = get_interpolated_velocity(world_pos);
            fvec2 comes_from = {world_pos.x - v.x * TIMESTEP, world_pos.y - v.y * TIMESTEP};

            fvec2 vx_uv = {comes_from.x / CELL_SIZE - 0.5f, comes_from.y / CELL_SIZE};
            smoke.velocitiesy[1-smoke.vy_idx][x][y] = sample_bilinear(&smoke.velocitiesy[smoke.vy_idx][0], smoke.size.x, smoke.size.y+1, vx_uv.x, vx_uv.y);;
        }
    }
}

void apply_temperature() {
    for (int x = 0; x < smoke.size.x; ++x) {
        for (int y = 0; y <= smoke.size.y; ++y) {
            if (is_solid((ivec2){x,y}) || is_solid((ivec2){x,y-1})) {
                smoke.velocitiesy[1-smoke.vy_idx][x][y] = 0.0f;
                continue;
            };

            fvec2 pos = (fvec2){ x + 0.5f, y};
            float temperature = sample_bilinear(&smoke.temp[smoke.s_idx][0], smoke.size.x, smoke.size.y, pos.x, pos.y);
            fvec3 smoke_data = sample_bilinear_fvec3(&smoke.smoke[smoke.s_idx][0], smoke.size.x, smoke.size.y, pos.x, pos.y);

            float smoke_amount = smoke_data.x + smoke_data.y + smoke_data.z;

            const float WEIGHT_F = 0.1f;
            const float TEMPERATURE_F = 1.0f;

            float bouyancy = - WEIGHT_F * smoke_amount + TEMPERATURE_F * (temperature - T_AMBIENT);
            smoke.velocitiesy[1-smoke.vy_idx][x][y] = smoke.velocitiesy[smoke.vy_idx][x][y] + bouyancy * TIMESTEP;
        }
    }
}

void handle_emitter() {
    if (!is_solid(e1.location)) {
        smoke.smoke[smoke.s_idx][e1.location.x][e1.location.y] = e1.densities;
        smoke.temp[smoke.s_idx][e1.location.x][e1.location.y] = e1.temperature;
    }

    if (!is_solid(e2.location)) {
        smoke.smoke[smoke.s_idx][e2.location.x][e2.location.y] = e2.densities;
        smoke.temp[smoke.s_idx][e2.location.x][e2.location.y] = e2.temperature;
    }
}

fvec2 get_interpolated_velocity(fvec2 p) {
    fvec2 v;

    float gx = p.x / CELL_SIZE;
    float gy = p.y / CELL_SIZE;

    v.x = sample_bilinear(&smoke.velocitiesx[smoke.vx_idx][0], smoke.size.x+1, smoke.size.y, gx, gy-0.5f);
    v.y = sample_bilinear(&smoke.velocitiesy[smoke.vy_idx][0], smoke.size.x, smoke.size.y+1, gx-0.5f, gy);

    return v;
}

void update_solid_map() {
    const int r_squared = o.radius * o.radius;

    for (int x = 0; x < smoke.size.x; ++x) {
        for (int y = 0; y < smoke.size.y; ++y) {
            if (x == 0 || y == 0 || x == smoke.size.x-1 || y == smoke.size.y-1) {
                smoke.solid[x][y] = 1;
                continue;
            }

            int dx = (x - o.location.x);
            int dy = (y - o.location.y);

            if ((dx*dx + dy*dy) <= r_squared) {
                smoke.solid[x][y] = 1;
            } else {
                smoke.solid[x][y] = 0;
            }
        }
    }
}

void handle_input(ivec2 input) {
    o.location.x += input.x;
    o.location.y += input.y;
}