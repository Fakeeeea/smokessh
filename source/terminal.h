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

void detect_size(ivec2* size);

void setup_terminal();
void reset_terminal();
ivec2 parse_input();

void set_handlers();

int should_resize();
void toggle_resize();

#endif //SMOKESSH_TERMINAL_H
