//
// Created by aaa on 22/10/2026.
//

#include "emitters.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>

void parse_emitters(smoke_config config, emitter** emitter_array, int* emitter_counter) {
    FILE* fptr;

    if (config.emitter_file == NULL) {
        config.emitter_file = "emitters_def";
    }

    if ((fptr = fopen(config.emitter_file, "r")) != NULL) {
        if (fscanf(fptr, "C:%d", emitter_counter) == 1) {

            MALLOC(*emitter_array, (*emitter_counter * sizeof(emitter)));

            for (int i = 0; i < *emitter_counter; ++i) {

                fscanf(fptr, " D:%f,%f,%f;T:%f;P:%d,%d", &(*emitter_array)[i].densities.x, &(*emitter_array)[i].densities.y, &(*emitter_array)[i].densities.z,
                    &(*emitter_array)[i].temperature,
                    &(*emitter_array)[i].location.x, &(*emitter_array)[i].location.y);
            }

        } else {
            *emitter_counter = 0;
        }

        fclose(fptr);

    } else {
        *emitter_counter = 0;
    }
}

void free_emitters(emitter** emitters, int* e_num) {
    if (*emitters != NULL) {
        free(*emitters);
        *emitters = NULL;
        *e_num = 0;
    }
}