//
// Created by aaa on 19/10/2026.
//

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
    char* end_ptr;

    while ((opt = getopt(argc, argv, "f:e:t:s:c:o:w:")) != -1) {
        switch (opt) {
            case 'f':
                config.target_fps = (int)strtol(optarg, &end_ptr, 10);
                config.target_fps = config.target_fps < 1 ? 1 : config.target_fps;
                break;
            case 'e':
                config.emitter_file = optarg;
                break;
            case 't':
                config.t_ambient = (int)strtol(optarg, &end_ptr, 10);
                break;
            case 's':
                config.time_step = strtof(optarg, &end_ptr);
                config.time_step = config.time_step <= 0 ? 0.0167f : config.time_step;
                break;
            case 'c':
                config.cell_size = strtof(optarg, &end_ptr);
                config.cell_size = config.cell_size <= 0 ? 0.1f : config.cell_size;
                break;
            case 'o':
                config.obstacle_radius = (int)strtol(optarg, &end_ptr, 10);
                config.obstacle_radius = config.obstacle_radius < 0 ? 0 : config.obstacle_radius;
                break;
            case 'w':
                config.wind = strtof(optarg, &end_ptr);
                break;

            default:
                fprintf(stderr, "Usage %s [-f target_fps]\n [-e emitters_file ('-' stdin)]\n [-t t_ambient]\n [-s time_step]\n [-c cell_size]\n [-o obstacle_radius]\n [-w wind]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char *argv[]) {
    app a;
    parse_flags(argc, argv);
    /*set_handlers();
    ivec2 size = detect_size();

    init_graphics();
    sim = init_simulation(config, size);
    register_simulation(&sim);

    if (!isatty(STDIN_FILENO)) reattach_tty();

    setup_terminal();
    set_atexit();*/

    app_init(&a, config);

    // const long target_frame_time_us = (long)(1000000.0f / (float)config.target_fps);
    //
    // struct timespec start_time, end_time;
    //
    // while (should_keep_running()) {
    //
    //     if (should_resize()) {
    //         size = detect_size();
    //         restart_simulation(&sim, size);
    //         init_graphics();
    //     }
    //
    //     clock_gettime(CLOCK_MONOTONIC, &start_time);
    //
    //     input in = parse_input();
    //
    //     if (in.code == QUIT) { break;}
    //
    //     handle_obstacle_movement(&sim.obstacle, in.mov, size);
    //     run(&sim);
    //     print_smoke(&sim.env);
    //
    //     clock_gettime(CLOCK_MONOTONIC, &end_time);
    //
    //     long elapsed_us = (end_time.tv_sec - start_time.tv_sec) * 1000000L + (end_time.tv_nsec - start_time.tv_nsec) / 1000L;
    //     if (elapsed_us < target_frame_time_us) {
    //         usleep(target_frame_time_us - elapsed_us);
    //     }
    // }

    app_run(&a);

    //app_shutdown(&a);
    /* atexit calls app_shutdown already, so it's commented out here.
     */
    return 0;
}