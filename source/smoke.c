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
#define CELL_SIZE 0.1f
#define TIMESTEP (0.0167f) // 1/60

// placeholder values
struct winsize terminal_size = {
    .ws_col = 30,
    .ws_row = 100
};

environment smoke;

obstacle player;

emitter e = {
    .location = {50, 3},
    .densities = {70, 10, 10},
    .temperature = 300,
};

void run() {
    handle_emitter();
    //handle player

    advect_vx(), advect_vy();
    smoke.vx_idx = 1 - smoke.vx_idx, smoke.vy_idx = 1 - smoke.vy_idx;

    apply_temperature();
    smoke.vy_idx = 1 - smoke.vy_idx;

    for (int x = 0; x < smoke.size.x; ++x) {
        for (int y = 0; y < smoke.size.y; ++y) {
            smoke.pressure[x][y] = 0;
        }
    }

    solve_pressure();
    update_vx(), update_vy();
    smoke.vx_idx = 1 - smoke.vx_idx, smoke.vy_idx = 1 - smoke.vy_idx;

    advect_smoke();
    smoke.s_idx = 1 - smoke.s_idx;
    print_smoke();
}

void init_smoke() {
    smoke.size = (ivec2){terminal_size.ws_row, terminal_size.ws_col};

    smoke.velocitiesx[0] = malloc((smoke.size.x + 1) * sizeof(float*));
    for (int i = 0; i <= smoke.size.x; i++) {
        smoke.velocitiesx[0][i] = malloc(smoke.size.y * sizeof(float));
    }

    smoke.velocitiesx[1] = malloc((smoke.size.x + 1) * sizeof(float*));
    for (int i = 0; i <= smoke.size.x; i++) {
        smoke.velocitiesx[1][i] = malloc(smoke.size.y * sizeof(float));
    }

    smoke.velocitiesy[0] = malloc(smoke.size.x * sizeof(float*));
    for (int i = 0; i < smoke.size.x; i++) {
        smoke.velocitiesy[0][i] = malloc((smoke.size.y + 1) * sizeof(float));
    }

    smoke.velocitiesy[1] = malloc(smoke.size.x * sizeof(float*));
    for (int i = 0; i < smoke.size.x; i++) {
        smoke.velocitiesy[1][i] = malloc((smoke.size.y + 1) * sizeof(float));
    }

    smoke.pressure = malloc(smoke.size.x * sizeof(float*));
    for (int i = 0; i < smoke.size.x; i++) {
        smoke.pressure[i] = malloc(smoke.size.y * sizeof(float));
    }

    smoke.smoke[0] = malloc(smoke.size.x * sizeof(float*));
    for (int i = 0; i < smoke.size.x; i++) {
        smoke.smoke[0][i] = malloc(smoke.size.y * sizeof(float));
    }

    smoke.temp[0] = malloc(smoke.size.x * sizeof(float*));
    for (int i = 0; i < smoke.size.x; i++) {
        smoke.temp[0][i] = malloc(smoke.size.y * sizeof(float));
    }

    smoke.smoke[1] = malloc(smoke.size.x * sizeof(float*));
    for (int i = 0; i < smoke.size.x; i++) {
        smoke.smoke[1][i] = malloc(smoke.size.y * sizeof(float));
    }

    smoke.temp[1] = malloc(smoke.size.x * sizeof(float*));
    for (int i = 0; i < smoke.size.x; i++) {
        smoke.temp[1][i] = malloc(smoke.size.y * sizeof(float));
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

            smoke.smoke[0][x][y] = 0;
            smoke.smoke[1][x][y] = 0;
            smoke.temp[0][x][y] = T_AMBIENT;
            smoke.temp[1][x][y] = T_AMBIENT;

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

        free(smoke.smoke[0][x]);
        free(smoke.smoke[1][x]);
        free(smoke.temp[0][x]);
        free(smoke.temp[1][x]);

        free(smoke.velocitiesy[0][x]);
        free(smoke.velocitiesy[1][x]);
    }
    free(smoke.smoke[0]);
    free(smoke.temp[0]);
    free(smoke.smoke[1]);
    free(smoke.temp[1]);

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

int print_smoke() {
    const char levels[] = " .:-=+*#%@";
    int n_levels = sizeof(levels) -1;

    printf("\x1b[H");
    for (int y = smoke.size.y - 1; y >= 0; y--) {
        for (int x = 0; x < smoke.size.x; ++x) {

            if (is_solid((ivec2){x,y})) {putchar('&'); continue;}

            int level_idx = (int)(smoke.smoke[smoke.s_idx][x][y]);
            if (level_idx < 0) level_idx = 0;
            if (level_idx >= n_levels) level_idx = n_levels -1;

            putchar(levels[level_idx]);
        }
        putchar('\n');
    }
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
    for (int x = 0; x < smoke.size.x; ++x) {
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
        for (int y = 0; y < smoke.size.y; ++y) {
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
                smoke.smoke[1-smoke.s_idx][x][y] = 0;
                smoke.temp[1-smoke.s_idx][x][y] = T_AMBIENT;
                continue;
            };

            fvec2 world_pos = {(x + 0.5f) * CELL_SIZE, (y + 0.5f) * CELL_SIZE};
            fvec2 v = get_interpolated_velocity(world_pos);
            fvec2 comes_from = {world_pos.x - v.x * TIMESTEP, world_pos.y - v.y * TIMESTEP};

            fvec2 grid_space = {comes_from.x / CELL_SIZE - 0.5f, comes_from.y / CELL_SIZE - 0.5f};
            fvec2 prev = {grid_space.x, grid_space.y};
            smoke.smoke[1-smoke.s_idx][x][y] = sample_bilinear(&smoke.smoke[smoke.s_idx][0], smoke.size.x, smoke.size.y, prev.x, prev.y);
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
            float smoke_amount = sample_bilinear(&smoke.smoke[smoke.s_idx][0], smoke.size.x, smoke.size.y, pos.x, pos.y);

            const float WEIGHT_F = 0.1f;
            const float TEMPERATURE_F = 1.0f;

            float bouyancy = - WEIGHT_F * smoke_amount + TEMPERATURE_F * (temperature - T_AMBIENT);
            smoke.velocitiesy[1-smoke.vy_idx][x][y] = smoke.velocitiesy[smoke.vy_idx][x][y] + bouyancy * TIMESTEP;
        }
    }
}

//TODO: ADD COLORS
void handle_emitter() {
    smoke.smoke[smoke.s_idx][e.location.x][e.location.y] = e.densities.x;
    smoke.temp[smoke.s_idx][e.location.x][e.location.y] = e.temperature;
}

fvec2 get_interpolated_velocity(fvec2 p) {
    fvec2 v;

    float gx = p.x / CELL_SIZE;
    float gy = p.y / CELL_SIZE;

    v.x = sample_bilinear(&smoke.velocitiesx[smoke.vx_idx][0], smoke.size.x+1, smoke.size.y, gx, gy-0.5f);
    v.y = sample_bilinear(&smoke.velocitiesy[smoke.vy_idx][0], smoke.size.x, smoke.size.y+1, gx-0.5f, gy);

    return v;
}

int main() {
    get_terminal_size(&terminal_size);
    init_smoke();
    reset_smoke();

    while (1) {
        run();
    }
    return 0;
}