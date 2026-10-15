//
// Created by aaa on 15/10/2026.
//

#ifndef SMOKESSH_TERMINAL_H
#define SMOKESSH_TERMINAL_H

#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

void error(char *msg);
int get_terminal_size(struct winsize *terminal_size);

#endif //SMOKESSH_TERMINAL_H
