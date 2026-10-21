//
// Created by aaa on 19/10/2026.

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "config.h"
#include "smoke.h"
#include "terminal.h"
#include "parser.h"
#include "graphics.h"
#include "physics.h"

smoke_config config = {
    .target_fps = 30,
    .emitter_file = NULL,

    .t_ambient = T_AMBIENT,
    .cell_size = CELL_SIZE,
    .time_step = TIMESTEP,
};

void parse_flags(int argc, char *argv[]) {
    int opt;

    while ((opt = getopt(argc, argv, "f:e:t:s:c:")) != -1) {
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
            default:
                fprintf(stderr, "Usage %s [-f target_fps]\n [-e emitters_file]\n [-t t_ambient]\n [-s time_step]\n [-c cell_size]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char *argv[]) {
    parse_flags(argc, argv);
    set_handlers();
    detect_size(&size);
    init_smoke();
    init_obstacle();
    init_graphics();
    reset_smoke();
    parse_emitters(&e, &e_num);
    setup_terminal();

    const long target_frame_time_us = (long)(1000000.0f / (float)config.target_fps);

    struct timespec start_time, end_time;

    while (1) {

        if (smoke.resize_pending) {
            restart_simulation();
        }

        clock_gettime(CLOCK_MONOTONIC, &start_time);

        ivec2 mov = parse_input();

        if (mov.x == QUIT && mov.y == QUIT) {
            break;
        }

        handle_input(mov);
        run();
        print_smoke();

        clock_gettime(CLOCK_MONOTONIC, &end_time);

        long elapsed_us = (end_time.tv_sec - start_time.tv_sec) * 1000000L + (end_time.tv_nsec - start_time.tv_nsec) / 1000L;
        if (elapsed_us < target_frame_time_us) {
            usleep(target_frame_time_us - elapsed_us);
        }
    }

    reset_terminal();
    show_cursor();
    free_smoke();
    return 0;
}