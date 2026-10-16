//
// Created by aaa on 16/10/2026.
//

#include "smoke_parser.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void parse_emitters(emitter** emitter_array, int* emitter_counter) {
    FILE* fptr;

    if ((fptr = fopen("emitters", "r")) != NULL) {
        if (fscanf(fptr, "C:%d", emitter_counter)) {

            *emitter_array = malloc(*emitter_counter * sizeof(emitter));

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