//
// Created by aaa on 28/10/2026.
//

#ifndef SMOKESSH_MENU_H
#define SMOKESSH_MENU_H

#include "config.h"
#include "menu_components.h"

typedef enum menu_window {
    PAGE_MAIN,
    PAGE_SETTINGS,
    PAGE_CONFIG,
    PAGE_EMITTERS,
    PAGE_COUNT
} menu_window;

#define MAX_DEPTH 8

typedef struct menu {
    smoke_config* config;

    page pages[PAGE_COUNT];
    int stack[MAX_DEPTH];
    int depth;

    int result;
}menu;

void init_menu(menu* i_menu, smoke_config* config);
int update_menu(int code);
void draw_menu(ivec2 size);

#endif //SMOKESSH_MENU_H
