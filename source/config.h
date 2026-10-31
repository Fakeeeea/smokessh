//
// Created by aaa on 19/10/2026.
//

#ifndef SMOKESSH_CONFIG_H
#define SMOKESSH_CONFIG_H

#include "limits.h"

typedef struct smoke_config {
    int target_fps;

    char* emitter_file;
    char emitter_dir[PATH_MAX];

    int t_ambient;
    int obstacle_radius;
    float cell_size;
    float time_step;
    float wind;
}smoke_config;

#endif //SMOKESSH_CONFIG_H