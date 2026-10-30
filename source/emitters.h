//
// Created by aaa on 22/10/2026.
//

#ifndef SMOKESSH_EMITTERS_H
#define SMOKESSH_EMITTERS_H

#include "sim_math.h"
#include "config.h"

typedef struct emitter {
    ivec2 location;
    fvec3 densities;
    float temperature;
}emitter;

void parse_emitters(smoke_config config, emitter** emitters, int* e_num, ivec2 size);
void free_emitters(emitter** emitters, int* e_num);

#endif //SMOKESSH_EMITTERS_H
