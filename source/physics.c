//
// Created by aaa on 16/10/2026.
//

#include "physics.h"
#include "config.h"
#include "utils.h"

int is_solid(const environment* smoke, const ivec2 p) {
    if (p.x < 0 || p.y < 0 || p.x >= smoke->size.x || p.y >= smoke->size.y) {
        return 1;
    }

    return smoke->solid[IDX(p.x,p.y,smoke->size.x)];
}

float get_vx(const environment* smoke, const ivec2 p) {
    if (p.x < 0 || p.y < 0 || p.x >= (smoke->size.x + 1) || p.y >= smoke->size.y) {
        return 0;
    }

    return smoke->velocitiesx[smoke->vx_idx][IDX(p.x,p.y,smoke->size.x+1)];
}

float get_vy(const environment* smoke, const ivec2 p) {
    if (p.x < 0 || p.y < 0 || p.x >= smoke->size.x || p.y >= (smoke->size.y + 1)) {
        return 0;
    }

    return smoke->velocitiesy[smoke->vy_idx][IDX(p.x,p.y,smoke->size.x)];
}

float get_pressure(const environment* smoke, const ivec2 p) {
    if (p.x < 0 || p.y < 0 || p.x >= smoke->size.x || p.y >= smoke->size.y) {
        return 0;
    }

    return smoke->pressure[IDX(p.x,p.y,smoke->size.x)];
}

void solve_pressure_iteration(environment* smoke, const smoke_config config) {
    for (int y = 0; y < smoke->size.y; ++y) {
        for (int x = 0; x < smoke->size.x; ++x) {
            int solid_left = is_solid(smoke, (ivec2){x-1, y});
            int solid_right = is_solid(smoke, (ivec2){x+1, y});
            int solid_top = is_solid(smoke, (ivec2){x, y+1});
            int solid_bottom = is_solid(smoke, (ivec2){x, y-1});

            int flow_left = 1 - solid_left;
            int flow_right = 1 - solid_right;
            int flow_top = 1 - solid_top;
            int flow_bottom = 1 - solid_bottom;

            int f_sum = flow_left + flow_right + flow_top + flow_bottom;
            if (f_sum == 0) continue;

            float v_left = get_vx(smoke, (ivec2){x,y});
            float v_right = get_vx(smoke, (ivec2){x+1,y});
            float v_bottom = get_vy(smoke, (ivec2){x,y});
            float v_top = get_vy(smoke, (ivec2){x,y+1});

            float delta_v = v_right - v_left + v_top - v_bottom;

            float p_left = get_pressure(smoke, (ivec2){x-1,y}) * (float) flow_left;
            float p_right = get_pressure(smoke, (ivec2){x+1,y}) * (float) flow_right;
            float p_top = get_pressure(smoke, (ivec2){x,y+1}) * (float) flow_top;
            float p_bottom = get_pressure(smoke, (ivec2){x,y-1}) * (float) flow_bottom;

            float p_sum = p_left + p_right + p_top + p_bottom;
            float p = (p_sum - config.cell_size * delta_v / config.time_step) / (float) f_sum;

            smoke->pressure[IDX(x,y,smoke->size.x)] = p;
        }
    }
}

void solve_pressure(environment* smoke, const smoke_config config) {
    const int ITERATIONS = 60;
    for (int i = 0; i < ITERATIONS; ++i) {
        solve_pressure_iteration(smoke, config);
    }
}
void update_vx(environment* smoke, const smoke_config config) {
    const float k = - config.time_step / config.cell_size;
    for (int y = 0; y < smoke->size.y; ++y) {
        for (int x = 0; x <= smoke->size.x; ++x) {

            const int idx = IDX(x,y,smoke->size.x+1);
            if (is_solid(smoke, (ivec2){x,y}) || is_solid(smoke, (ivec2){x-1,y})) {
                smoke->velocitiesx[1-smoke->vx_idx][idx] = 0;
                continue;
            }
            float p_right = get_pressure(smoke, (ivec2){x,y});
            float p_left = get_pressure(smoke, (ivec2){x-1,y});

            smoke->velocitiesx[1-smoke->vx_idx][idx] = smoke->velocitiesx[smoke->vx_idx][idx] + k * (p_right - p_left);
        }
    }
}
void update_vy(environment* smoke, const smoke_config config) {
    const float k = - config.time_step / config.cell_size;
    for (int y = 0; y <= smoke->size.y; ++y) {
        for (int x = 0; x < smoke->size.x; ++x) {

            const int idx = IDX(x,y,smoke->size.x);
            if (is_solid(smoke, (ivec2){x,y}) || is_solid(smoke, (ivec2){x,y-1})) {
                smoke->velocitiesy[1-smoke->vy_idx][idx] = 0;
                continue;
            }

            float p_top = get_pressure(smoke, (ivec2){x,y});
            float p_bottom = get_pressure(smoke, (ivec2){x,y-1});

            smoke->velocitiesy[1-smoke->vy_idx][idx] = smoke->velocitiesy[smoke->vy_idx][idx] + k * (p_top - p_bottom);
        }
    }
}

void advect_smoke(environment* smoke, const smoke_config config) {
    for (int y = 0; y < smoke->size.y; ++y) {
        for (int x = 0; x < smoke->size.x; ++x) {
            const int idx = IDX(x,y,smoke->size.x);

            if (is_solid(smoke, (ivec2){ x,  y})) {
                smoke->smoke[1-smoke->s_idx][idx] = (fvec3){0,0,0};
                smoke->temp[1-smoke->s_idx][idx] = (float)config.t_ambient;
                continue;
            };

            const fvec2 world_pos = {((float)x + 0.5f) * config.cell_size, ((float)y + 0.5f) * config.cell_size};
            const fvec2 v = get_interpolated_velocity(smoke, config, world_pos);
            const fvec2 comes_from = {world_pos.x - v.x * config.time_step, world_pos.y - v.y * config.time_step};

            const fvec2 grid_space = {comes_from.x / config.cell_size - 0.5f, comes_from.y / config.cell_size - 0.5f};
            const fvec2 prev = {grid_space.x, grid_space.y};

            smoke->smoke[1-smoke->s_idx][idx] = sample_bilinear_fvec3(smoke->smoke[smoke->s_idx], smoke->size.x, smoke->size.y, prev.x, prev.y);
            smoke->temp[1-smoke->s_idx][idx] = sample_bilinear(smoke->temp[smoke->s_idx], smoke->size.x, smoke->size.y, prev.x, prev.y);;
        }
    }
}

void advect_vx(environment* smoke, const smoke_config config) {
    for (int y = 0; y < smoke->size.y; ++y) {
        for (int x = 0; x <= smoke->size.x; ++x) {

            const int idx = IDX(x,y,smoke->size.x+1);
            if (is_solid(smoke, (ivec2){x, y}) || is_solid(smoke, (ivec2){ x-1, y})) {
                smoke->velocitiesx[1-smoke->vx_idx][idx] = 0.0f;
                continue;
            }

            const fvec2 world_pos = { (float)x * config.cell_size, ((float)y + 0.5f) * config.cell_size};
            const fvec2 v = get_interpolated_velocity(smoke, config, world_pos);
            const fvec2 comes_from = {world_pos.x - v.x * config.time_step, world_pos.y - v.y * config.time_step};

            const fvec2 vx_uv = {comes_from.x / config.cell_size, comes_from.y / config.cell_size - 0.5f};

            smoke->velocitiesx[1-smoke->vx_idx][idx] = sample_bilinear(smoke->velocitiesx[smoke->vx_idx], smoke->size.x+1, smoke->size.y, vx_uv.x, vx_uv.y);;
        }
    }
}
void advect_vy(environment* smoke, const smoke_config config) {
    for (int y = 0; y <= smoke->size.y; ++y) {
        for (int x = 0; x < smoke->size.x; ++x) {

            const int idx = IDX(x,y,smoke->size.x);
            if (is_solid(smoke, (ivec2){ x, y}) || is_solid(smoke, (ivec2){ x, y-1})) {
                smoke->velocitiesy[1-smoke->vy_idx][idx] = 0.0f;
                continue;
            }

            const fvec2 world_pos = {((float)x + 0.5f) * config.cell_size, (float)y * config.cell_size};
            const fvec2 v = get_interpolated_velocity(smoke, config, world_pos);
            const fvec2 comes_from = {world_pos.x - v.x * config.time_step, world_pos.y - v.y * config.time_step};

            const fvec2 vx_uv = {comes_from.x / config.cell_size - 0.5f, comes_from.y / config.cell_size};

            smoke->velocitiesy[1-smoke->vy_idx][idx] = sample_bilinear(smoke->velocitiesy[smoke->vy_idx], smoke->size.x, smoke->size.y+1, vx_uv.x, vx_uv.y);;
        }
    }
}

void apply_temperature(environment* smoke, const smoke_config config) {
    for (int y = 0; y <= smoke->size.y; ++y) {
        for (int x = 0; x < smoke->size.x; ++x) {
            const int idx = IDX(x,y,smoke->size.x);

            if (is_solid(smoke, (ivec2){x,y}) || is_solid(smoke, (ivec2){x,y-1})) {
                smoke->velocitiesy[1-smoke->vy_idx][idx] = 0.0f;
                continue;
            };

            fvec2 pos = (fvec2){ (float) x + 0.5f, (float) y};
            float temperature = sample_bilinear(smoke->temp[smoke->s_idx], smoke->size.x, smoke->size.y, pos.x, pos.y);
            fvec3 smoke_data = sample_bilinear_fvec3(smoke->smoke[smoke->s_idx], smoke->size.x, smoke->size.y, pos.x, pos.y);

            float smoke_amount = smoke_data.x + smoke_data.y + smoke_data.z;

            const float WEIGHT_F = 0.1f;
            const float TEMPERATURE_F = 1.0f;

            float buoyancy = - WEIGHT_F * smoke_amount + TEMPERATURE_F * (temperature - (float)config.t_ambient);

            smoke->velocitiesy[1-smoke->vy_idx][idx] = smoke->velocitiesy[smoke->vy_idx][idx] + buoyancy * config.time_step;
        }
    }
}

void handle_emitter(environment* smoke, const emitter* emitters, const int e_num) {
    for (int i = 0; i < e_num; ++i) {
        emitter e = emitters[i];
        if (!is_solid(smoke, e.location)) {

            const int idx = IDX(e.location.x,e.location.y,smoke->size.x);
            smoke->smoke[smoke->s_idx][idx] = e.densities;
            smoke->temp[smoke->s_idx][idx] = e.temperature;
        }
    }
}

fvec2 get_interpolated_velocity(const environment* smoke, const smoke_config config, fvec2 p) {
    fvec2 v;

    float gx = p.x / config.cell_size;
    float gy = p.y / config.cell_size;

    v.x = sample_bilinear(smoke->velocitiesx[smoke->vx_idx], smoke->size.x+1, smoke->size.y, gx, gy-0.5f);
    v.y = sample_bilinear(smoke->velocitiesy[smoke->vy_idx], smoke->size.x, smoke->size.y+1, gx-0.5f, gy);

    return v;
}

void update_solid_map(environment* smoke, const obstacle o) {
    const int r_squared = o.radius * o.radius;

    for (int y = 0; y < smoke->size.y; ++y) {
        for (int x = 0; x < smoke->size.x; ++x) {
            const int idx = IDX(x,y,smoke->size.x);

            if (x == 0 || y == 0 || x == smoke->size.x-1 || y == smoke->size.y-1) {
                smoke->solid[idx] = 1;
                continue;
            }

            int dx = (x - o.location.x);
            int dy = (y - o.location.y);

            if ((dx*dx + dy*dy) <= r_squared) {
                smoke->solid[idx] = 1;
            } else {
                smoke->solid[idx] = 0;
            }
        }
    }
}