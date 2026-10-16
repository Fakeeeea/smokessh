//
// Created by aaa on 15/10/2026.
//

#ifndef SMOKESSH_TERMINAL_H
#define SMOKESSH_TERMINAL_H

#include "smoke_math.h"

void error(char *msg);

void setup_terminal();
void reset_terminal();
ivec2 parse_input();

#endif //SMOKESSH_TERMINAL_H
