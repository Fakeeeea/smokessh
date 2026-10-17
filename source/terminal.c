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

#define DEFAULT_SIZE_X 100
#define DEFAULT_SIZE_Y 30

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

void detect_size(ivec2* size) {
    struct winsize ws;

    if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) == 0) {
        *size = (ivec2){ws.ws_col, ws.ws_row};
        if (size->x < 10) size->x = 10;
        if (size->y < 10) size->y = 10;

        if (size->x > 256) size->x = 256;
        if (size->y > 256) size->y = 256;
    } else {
        *size = (ivec2){DEFAULT_SIZE_X, DEFAULT_SIZE_Y};
    }
}

extern ivec2 size;
extern environment smoke;

void resize_handler(int sig) {
    struct winsize ws;

    if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) == 0) {
        size = (ivec2){ws.ws_col, ws.ws_row};
        
        if (size.x < 10) size.x = 10;
        if (size.y < 10) size.y = 10;

        if (size.x > 256) size.x = 256;
        if (size.y > 256) size.y = 256;
    } else {
        size = (ivec2){DEFAULT_SIZE_X, DEFAULT_SIZE_Y};
    }

    smoke.init = 0;
}