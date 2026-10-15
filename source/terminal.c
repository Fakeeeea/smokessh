//
// Created by aaa on 15/10/2026.
//

#include "terminal.h"

void error(char *msg) {
    perror(msg);
    exit(-1);
}

int get_terminal_size(struct winsize *terminal_size) {
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, terminal_size) != 0) {
        error("Failed fetching terminal size!");
    }
}