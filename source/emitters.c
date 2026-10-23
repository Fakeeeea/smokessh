//
// Created by aaa on 22/10/2026.
//

#include "emitters.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 256
#define DEFAULT_EMITTER_FILE "default"

void parse_emitters(smoke_config config, emitter** emitter_array, int* emitter_counter, ivec2 size) {
    *emitter_array = NULL;
    *emitter_counter = 0;

    const char* path = (config.emitter_file) ? config.emitter_file : DEFAULT_EMITTER_FILE;

    FILE* fptr = fopen(path, "r");
    if (!fptr) {
        fprintf(stderr, "emitters: cannot open '%s'\n", path);
        exit(EXIT_FAILURE);
    }

    emitter* arr = NULL;
    int count = 0, cap = 0;
    int percent = 0;
    char line[MAX_LINE];
    int lineno = 0;

    while (fgets(line, sizeof(line), fptr)) {
        lineno++;
        if (!strchr(line, '\n') && !feof(fptr)) {
            fprintf(stderr, "emitters:%d : line too long\n", lineno);
            goto fail;
        }

        char* hash = strchr(line, '#');
        if (hash) *hash = '\0';
        char *p = line;
        while (*p == ' ' || *p == 't') p++;
        if (*p == '\0' || *p == '\n' || *p == '\r') continue;

        char word[16], unit[16];
        if (sscanf(p, "%15s", word) != 1) continue;

        if (strcmp(word, "units") == 0) {
            if (sscanf(p, "%*s %15s", unit) != 1) {
                fprintf(stderr, "emitters:%d: expected 'units percent|cells'\n", lineno);
                goto fail;
            }
            if (strcmp(unit, "percent") == 0) percent = 1;
            else if (strcmp(unit, "cells") == 0) percent = 0;
            else {
                fprintf(stderr, "emitters:%d: unknown units '%s'\n", lineno, unit);
                goto fail;
            }
        } else if (strcmp(word, "emitter") == 0) {
            float x, y, d0, d1, d2, temp;
            if (sscanf(p, "%*s %f %f %f %f %f %f", &x, &y, &d0, &d1, &d2, &temp) != 6) {
                fprintf(stderr, "emitters:%d: expected 'emitter x y d0 d1 d2 temp'\n'", lineno);
                goto fail;
            }

            if (count == cap) {
                int new_cap = cap ? cap * 2 : 8;
                emitter* tmp = realloc(arr, (size_t)new_cap * sizeof(*arr));
                if (!tmp) {
                    fprintf(stderr, "mitters: out of memory\n");
                    goto fail;
                }
                arr = tmp;
                cap = new_cap;
            }

            emitter* e = &arr[count++];
            memset(e, 0, sizeof(*e));
            e->densities.x = d0, e->densities.y = d1, e->densities.z = d2;
            e->temperature = temp;
            if (percent) {
                e->location.x = (int)(size.x * x / 100.0f);
                e->location.y = (int)(size.y * y / 100.0f);
            } else {
                e->location.x = (int)x;
                e->location.y = (int)y;
            }

            if (e->location.x < 0 || e->location.x >= size.x ||
                e->location.y < 0 || e->location.y >= size.y) {
                fprintf(stderr, "emitters:%d: position outside grid\n", lineno);
                goto fail;
            }
        } else {
            fprintf(stderr, "emitters: read error on '%s'\n", path);
            goto fail;
        }
    }

    if (ferror(fptr)) {
        fprintf(stderr, "emitters: read error on '%s'\n", path);
        goto fail;
    }

    fclose(fptr);
    *emitter_array = arr;
    *emitter_counter = count;
    return;

    fail:
    fclose(fptr);
    free(arr);
    exit(EXIT_FAILURE);
}

void free_emitters(emitter** emitters, int* e_num) {
    FREE(*emitters);
    *e_num = 0;
}