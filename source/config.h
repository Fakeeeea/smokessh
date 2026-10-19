//
// Created by aaa on 19/10/2026.
//

#ifndef SMOKESSH_CONFIG_H
#define SMOKESSH_CONFIG_H

typedef struct smoke_config {
    int target_fps;
    char *emitter_file;

    int t_ambient;
    float cell_size;
    float time_step;
}smoke_config;

extern smoke_config config;

#endif //SMOKESSH_CONFIG_H
