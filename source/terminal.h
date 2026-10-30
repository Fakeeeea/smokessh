//
// Created by aaa on 15/10/2026.
//

#ifndef SMOKESSH_TERMINAL_H
#define SMOKESSH_TERMINAL_H

#include "sim_math.h"
#include "simulation.h"
#include "app.h"

#define DEFAULT_SIZE_X 100
#define DEFAULT_SIZE_Y 30

#define MOV 0

typedef enum action {ACT_NONE, ACT_UP, ACT_DOWN, ACT_LEFT, ACT_RIGHT, ACT_SELECT, ACT_QUIT} action;

typedef struct input {
    int code;
    ivec2 mov;
}input;

ivec2 detect_size();

void setup_terminal();
void reset_terminal();
action parse_input();

void set_handlers();

int should_resize();
int should_keep_running();

void reattach_tty();

void register_simulation(simulation *sim);
void register_app(app *a);
void set_atexit();

#endif //SMOKESSH_TERMINAL_H
