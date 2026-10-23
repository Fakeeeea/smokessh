//
// Created by aaa on 19/10/2026.

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "config.h"
#include "simulation.h"
#include "terminal.h"
#include "graphics.h"
#include "physics.h"

smoke_config config = {
    .target_fps = 30,
    .emitter_file = NULL,

    .t_ambient = T_AMBIENT,
    .cell_size = CELL_SIZE,
    .time_step = TIMESTEP,
    .obstacle_radius = INIT_RADIUS,
    .wind = 0,
};

simulation sim;

void parse_flags(int argc, char *argv[]) {
    int opt;

    while ((opt = getopt(argc, argv, "f:e:t:s:c:o:w:")) != -1) {
        switch (opt) {
            case 'f':
                config.target_fps = atoi(optarg);
                break;
            case 'e':
                config.emitter_file = optarg;
                break;
            case 't':
                config.t_ambient = atoi(optarg);
                break;
            case 's':
                config.time_step = atof(optarg);
                config.time_step = config.time_step <= 0 ? 0.0167f : config.time_step;
                break;
            case 'c':
                config.cell_size = atof(optarg);
                config.cell_size = config.cell_size <= 0 ? 0.1f : config.cell_size;
                break;
            case 'o':
                config.obstacle_radius = atoi(optarg);
                break;
            case 'w':
                config.wind = atof(optarg);
                break;
            default:
                fprintf(stderr, "Usage %s [-f target_fps]\n [-e emitters_file]\n [-t t_ambient]\n [-s time_step]\n [-c cell_size]\n [-o obstacle_radius]\n [-w wind]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char *argv[]) {
    ivec2 size;

    parse_flags(argc, argv);
    set_handlers();
    detect_size(&size);

    init_graphics();
    sim = init_simulation(config, size);
    setup_terminal();

    const long target_frame_time_us = (long)(1000000.0f / (float)config.target_fps);

    struct timespec start_time, end_time;

    while (1) {

        if (should_resize()) {
            detect_size(&size);
            restart_simulation(&sim, size);
            init_graphics();
        }

        clock_gettime(CLOCK_MONOTONIC, &start_time);

        ivec2 mov = parse_input();

        if (mov.x == QUIT && mov.y == QUIT) {
            break;
        }

        handle_obstacle_movement(&sim.obstacle, mov, size);
        run(&sim);
        print_smoke(&sim.env);

        clock_gettime(CLOCK_MONOTONIC, &end_time);

        long elapsed_us = (end_time.tv_sec - start_time.tv_sec) * 1000000L + (end_time.tv_nsec - start_time.tv_nsec) / 1000L;
        if (elapsed_us < target_frame_time_us) {
            usleep(target_frame_time_us - elapsed_us);
        }
    }

    free_simulation(&sim);
    reset_terminal();
    show_cursor();
    return 0;
}