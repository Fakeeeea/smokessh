//
// Created by aaa on 28/10/2026.
//

#ifndef SMOKESSH_APP_H
#define SMOKESSH_APP_H

#include "menu.h"
#include "simulation.h"

typedef enum app_state {
    APP_MENU,
    APP_SIM,
    APP_QUIT
} app_state;

typedef struct app {
    app_state state;

    smoke_config config;
    simulation sim;

    menu menu;
    ivec2 size;
}app;

void app_init(app* a, smoke_config config);
void app_run(app* a);
void app_shutdown(app* a);

#endif //SMOKESSH_APP_H
