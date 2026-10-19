//
// Created by aaa on 15/10/2026.
//

#include "terminal.h"

#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include "smoke.h"

static struct termios old, new;
static int oldf;

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
    ivec2 mov = {.x = 0, .y = 0};
    char c;
    while ((c = (char)getchar()) != EOF) {
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
                mov.y = mov.x = QUIT;
            default:
                break;
        }
    }

    return mov;
}

extern environment smoke;

void detect_size(ivec2* size) {
    struct winsize ws;

    if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) == 0) {
        *size = (ivec2){.x = ws.ws_col, .y = ws.ws_row};
        if (size->x < MIN_ANY_SIZE) size->x = MIN_ANY_SIZE;
        if (size->y < MIN_ANY_SIZE) size->y = MIN_ANY_SIZE;

        if (size->x > MAX_ANY_SIZE) size->x = MAX_ANY_SIZE;
        if (size->y > MAX_ANY_SIZE) size->y = MAX_ANY_SIZE;
    } else {
        *size = (ivec2){.x = DEFAULT_SIZE_X, .y = DEFAULT_SIZE_Y};
    }

    size->y--; //first line dedicated to header

    smoke.resize_pending = 0;
}

void resize_handler(int sig) {
    smoke.resize_pending = 1;
}