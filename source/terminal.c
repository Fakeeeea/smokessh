//
// Created by aaa on 15/10/2026.
//

#include "terminal.h"

#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <stdlib.h>

#include "simulation.h"

static struct termios old, new;
static int oldf;

static sig_atomic_t resize_pending = 0;
struct sigaction sa;

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

void reattach_tty() {
    int fd = open("/dev/tty", O_RDONLY);
    if (fd < 0) exit(EXIT_FAILURE);
    if (dup2(fd, STDIN_FILENO) < 0) {close(fd); exit(EXIT_FAILURE);}
    close(fd);
    clearerr(stdin);
}

input parse_input() {
    input out = {
        .mov = (ivec2) {.x = 0, .y = 0},
        .code = MOV,
    };

    int c;
    while ((c = getchar()) != EOF) {
        switch (c) {
            case 'a':
            case 'A':
                out.mov.x--;
                break;
            case 'd':
            case 'D':
                out.mov.x++;
                break;
            case 'w':
            case 'W':
                out.mov.y++;
                break;
            case 's':
            case 'S':
                out.mov.y--;
                break;
            case 'q':
                out.code = QUIT;
            default:
                break;
        }
    }

    return out;
}

void detect_size(ivec2* size) {
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) {
        *size = (ivec2){.x = ws.ws_col, .y = ws.ws_row};
        if (size->x < MIN_ANY_SIZE) size->x = MIN_ANY_SIZE;
        if (size->y < MIN_ANY_SIZE) size->y = MIN_ANY_SIZE;

        if (size->x > MAX_ANY_SIZE) size->x = MAX_ANY_SIZE;
        if (size->y > MAX_ANY_SIZE) size->y = MAX_ANY_SIZE;
    } else {
        *size = (ivec2){.x = DEFAULT_SIZE_X, .y = DEFAULT_SIZE_Y};
    }

    size->y--; //first line dedicated to header

    resize_pending = 0;
}

static void resize_handler(int sig) {
    resize_pending = 1;
}

void set_handlers() {
    sa.sa_handler = resize_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    sigaction(SIGWINCH, &sa, NULL);
}

int should_resize() {
    return resize_pending;
}

void toggle_resize() {
    resize_pending = 1 - resize_pending;
}