//
// Created by aaa on 15/10/2026.
//

#include "terminal.h"

#include <unistd.h>
#include <stdio.h>
#include <termios.h>
#include <fcntl.h>

struct termios old, new;
int oldf;

void setup_terminal() {
    tcgetattr(STDIN_FILENO, &old);

    new = old;

    new.c_lflag &= ~(ICANON | ECHO);

    tcsetattr(STDIN_FILENO, TCSANOW, &new);

    oldf = fcntl(STDIN_FILENO, F_GETFL);

    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
}

void reset_terminal() {
    tcsetattr(STDIN_FILENO, TCSANOW, &old);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
}

ivec2 parse_input() {
    ivec2 mov = {0,0};
    char c;
    while ((c = getchar()) != EOF) {
        switch (c) {
            case 'a':
            case 'A':
                mov.x--;
                break;
            case 'd':
            case 'D':
                mov.x++;
                break;
            case 'w':
            case 'W':
                mov.y++;
                break;
            case 's':
            case 'S':
                mov.y--;
                break;
            case 'q':
                mov.y = mov.x = 42;
        }
    }

    return mov;
}