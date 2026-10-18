//
// Created by aaa on 15/10/2026.
//

#ifndef SMOKESSH_TERMINAL_H
#define SMOKESSH_TERMINAL_H

#include "smoke_math.h"

#define DEFAULT_SIZE_X 100
#define DEFAULT_SIZE_Y 30

#define QUIT 42

void detect_size(ivec2* size);
void resize_handler(int sig);

void setup_terminal();
void reset_terminal();
ivec2 parse_input();

#endif //SMOKESSH_TERMINAL_H
