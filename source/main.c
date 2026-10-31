//
// Created by aaa on 19/10/2026.
//

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "simulation.h"
#include "terminal.h"
#include "physics.h"

smoke_config config = {
    .target_fps = 30,
    .emitter_file = NULL,
    .emitter_dir = "./",

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

    while ((opt = getopt(argc, argv, "d:f:e:t:s:c:o:w:")) != -1) {
        switch (opt) {
            case 'd':
                strcpy(config.emitter_dir, optarg);
                break;
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
                fprintf(stderr, "Usage %s [-f target_fps]\n [-e emitters_file ('-' stdin)]\n [-d emitters_dir]\n [-t t_ambient]\n [-s time_step]\n [-c cell_size]\n [-o obstacle_radius]\n [-w wind]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
}

/*
 * Keeping "a" as a global var so it doesn't stop existing with main,
 * causing atexit to call free on non-existent memory.
 */
app a;


int main(int argc, char *argv[]) {
    parse_flags(argc, argv);

    app_init(&a, config);
    app_run(&a);

    //app_shutdown(&a);
    /* atexit calls app_shutdown already, so it's commented out here.
     */
    return 0;
}