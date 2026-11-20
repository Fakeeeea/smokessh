//
// Created by aaa on 15/10/2026.
//

#include "terminal.h"

#include <signal.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <stdlib.h>

#include "simulation.h"
#include "app.h"

static struct termios old, new;
static int oldf;

static volatile sig_atomic_t resize_pending = 0;
static volatile sig_atomic_t keep_running = 1;

static struct sigaction sa;
static struct sigaction sa_int;

static int terminal_ready = 0;

static app *registered_app = NULL;

/*
 * Put STDIN into raw non blocking mode,
 * save old terminal settings.
 */
void setup_terminal() {
    tcgetattr(STDIN_FILENO, &old);

    new = old;

    new.c_lflag &= ~(ICANON | ECHO);

    tcsetattr(STDIN_FILENO, TCSANOW, &new);

    oldf = fcntl(STDIN_FILENO, F_GETFL);

    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    terminal_ready = 1;
}

/*
 * reset terminal to old settings if they have been changed
 */
void reset_terminal() {
    if (!terminal_ready) return;
    tcsetattr(STDIN_FILENO, TCSANOW, &old);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
}

/*
 * reconnect stdin back to tty
 */
void reattach_tty() {
    int fd = open("/dev/tty", O_RDONLY);
    if (fd < 0) exit(EXIT_FAILURE);
    if (dup2(fd, STDIN_FILENO) < 0) {close(fd); exit(EXIT_FAILURE);}
    close(fd);
    clearerr(stdin);
}

action parse_input() {
    action out = ACT_NONE;

    char c;
    ssize_t n;

    while ((n = read(STDIN_FILENO, &c, 1)) > 0) {

        if (c == '\033') {
            char seq[2];

            if (read(STDIN_FILENO, &seq[0], 1) <= 0) break;
            if (read(STDIN_FILENO, &seq[1], 1) <= 0) break;

            if (seq[0] == '[') {
                switch (seq[1]) {
                    case 'A': out = ACT_UP; break;
                    case 'B': out = ACT_DOWN; break;
                    case 'C': out = ACT_RIGHT; break;
                    case 'D': out = ACT_LEFT; break;
                    default: break;
                }
            }
        } else {
            switch (c) {
                case 'a': case 'A': out = ACT_LEFT; break;
                case 'd': case 'D': out = ACT_RIGHT; break;
                case 'w': case 'W': out = ACT_UP; break;
                case 's': case 'S': out = ACT_DOWN; break;
                case 'q': case 'Q': out = ACT_QUIT; break;
                case '\n': out = ACT_SELECT; break;
                default: break;
            }
        }
    }

    return out;
}

ivec2 detect_size() {
    ivec2 size;
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) {
        size = (ivec2){.x = ws.ws_col, .y = ws.ws_row};
        if (size.x < MIN_ANY_SIZE) size.x = MIN_ANY_SIZE;
        if (size.y < MIN_ANY_SIZE) size.y = MIN_ANY_SIZE;

        if (size.x > MAX_ANY_SIZE) size.x = MAX_ANY_SIZE;
        if (size.y > MAX_ANY_SIZE) size.y = MAX_ANY_SIZE;
    } else {
        size = (ivec2){.x = DEFAULT_SIZE_X, .y = DEFAULT_SIZE_Y};
    }

    resize_pending = 0;

    return size;
}

static void resize_handler(const int sig) {
    (void)sig;
    resize_pending = 1;
}

static void sigint_handler(const int sig) {
    (void)sig;
    keep_running = 0;
}

void register_app(app *a) {
    registered_app = a;
}

static void app_shutdown_wrapper() {
    app_shutdown(registered_app);
}

/*
 * set the 2 signal handlers:
 * - resize_handler for SIGWINCH (window resize)
 * - sigint_handler for SIGINT (ctrl + c)
 */
void set_handlers() {
    sa.sa_handler = resize_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    sigaction(SIGWINCH, &sa, NULL);

    sa_int.sa_handler = sigint_handler;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;

    sigaction(SIGINT, &sa_int, NULL);
}

/*
 * function to be called on program exit
 */
void set_atexit() {
    atexit(app_shutdown_wrapper);
}

int should_resize() {
    return resize_pending;
}

int should_keep_running() {
    return keep_running;
}