//
// Created by aaa on 15/10/2026.
//

#ifndef SMOKESSH_TERMINAL_H
#define SMOKESSH_TERMINAL_H

#include "smoke_math.h"

#define QUIT 42

void error(char *msg);

void detect_size(ivec2* size);
void resize_handler(int sig);

void setup_terminal();
void reset_terminal();
ivec2 parse_input();

#endif //SMOKESSH_TERMINAL_H
