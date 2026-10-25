//
// Created by aaa on 15/10/2026.
//

#ifndef SMOKESSH_TERMINAL_H
#define SMOKESSH_TERMINAL_H

#include "sim_math.h"
#include "simulation.h"

#define DEFAULT_SIZE_X 100
#define DEFAULT_SIZE_Y 30

#define QUIT 42
#define MOV 0

typedef struct input {
    int code;
    ivec2 mov;
}input;

void detect_size(ivec2* size);

void setup_terminal();
void reset_terminal();
input parse_input();

void set_handlers();

int should_resize();
int should_keep_running();

void reattach_tty();

void register_simulation(simulation *sim);
void set_atexit();

#endif //SMOKESSH_TERMINAL_H
