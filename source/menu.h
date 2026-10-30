//
// Created by aaa on 28/10/2026.
//

#ifndef SMOKESSH_MENU_H
#define SMOKESSH_MENU_H

#include "config.h"
#include "menu_components.h"

typedef enum menu_window {
    MENU_MAIN,
    MENU_SETTINGS,
    MENU_CONFIG,
    MENU_EMITTERS,
    MENU_NONE
} menu_window;

#define MENU_PAGES 4;

#define MENU_MAIN_WIDGETS 3
#define MENU_SETTINGS_WIDGETS 3
#define MENU_EMITTERS_WIDGETS 1
#define MENU_CONFIG_WIDGETS 6

typedef struct menu {
    menu_window window;
    smoke_config* config;

    page* pages;
    int p_count;
}menu;

void init_menu(menu* m, smoke_config* config);
int update_menu(menu* m, int code);
void draw_menu(const menu* m, ivec2 size);

#endif //SMOKESSH_MENU_H
