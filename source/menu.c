//
// Created by aaa on 28/10/2026.
//

#include <stdlib.h>
#include <stdio.h>

#include "menu.h"
#include "app.h"
#include "utils.h"
#include "graphics.h"
#include "terminal.h"

typedef enum menu_buttons {RUN, SETTINGS, QUIT, BACK, CONFIG, EMITTERS} menu_buttons;

void init_menu(menu* m, smoke_config* config) {
    m->window = MENU_MAIN;
    m->config = config;

    m->p_count = MENU_PAGES;
    MALLOC(m->pages, sizeof(page) * m->p_count);

    m->pages[MENU_MAIN].count = MENU_MAIN_WIDGETS;
    MALLOC(m->pages[MENU_MAIN].items, sizeof(widget) * MENU_MAIN_WIDGETS);

    m->pages[MENU_MAIN].title = "./smokessh\0";
    m->pages[MENU_MAIN].items[0] = create_button("Run", RUN);
    m->pages[MENU_MAIN].items[1] = create_button("Settings", SETTINGS);
    m->pages[MENU_MAIN].items[2] = create_button("Quit", QUIT);

    m->pages[MENU_SETTINGS].count = MENU_SETTINGS_WIDGETS;
    MALLOC(m->pages[MENU_SETTINGS].items, sizeof(widget) * MENU_SETTINGS_WIDGETS);

    m->pages[MENU_SETTINGS].title = "Settings\0";
    m->pages[MENU_SETTINGS].items[0] = create_button("Config", CONFIG);
    m->pages[MENU_SETTINGS].items[1] = create_button("Emitters preset", EMITTERS);
    m->pages[MENU_SETTINGS].items[2] = create_button("Back", BACK);

    m->pages[MENU_CONFIG].count = MENU_CONFIG_WIDGETS;
    MALLOC(m->pages[MENU_CONFIG].items, sizeof(widget) * MENU_CONFIG_WIDGETS);

    m->pages[MENU_CONFIG].title = "Config A/D - ARROWS\0";
    m->pages[MENU_CONFIG].items[0] = create_input_i("T Ambient", &config->t_ambient, -9999, 9999, 1);
    m->pages[MENU_CONFIG].items[1] = create_input_i("Obstacle radius", &config->obstacle_radius, 0, 9999, 1);
    m->pages[MENU_CONFIG].items[2] = create_input_f("Cell size", &config->cell_size, 0.01f, 1, 0.01f);
    m->pages[MENU_CONFIG].items[3] = create_input_f("Time step", &config->time_step, 0.001f, 1, 0.001f);
    m->pages[MENU_CONFIG].items[4] = create_input_f("Wind", &config->wind, -10, 10, 1);
    m->pages[MENU_CONFIG].items[5] = create_button("Back", BACK);
    //fps?

    m->pages[MENU_EMITTERS].count = MENU_EMITTERS_WIDGETS;
    MALLOC(m->pages[MENU_EMITTERS].items, sizeof(widget) * MENU_EMITTERS_WIDGETS);

    m->pages[MENU_EMITTERS].title = "Under construction!\0";
    m->pages[MENU_EMITTERS].items[0] = create_button("Back", BACK);
}

static int focus = 0;

int update_menu(menu* m, int code) {
    if (m->window == MENU_NONE) return 0;

    switch (code) {
        case ACT_QUIT:
            if (m->window == MENU_CONFIG || m->window == MENU_EMITTERS) {
                m->window = MENU_SETTINGS;
            } else if (m->window == MENU_SETTINGS) {
                m->window = MENU_MAIN;
            } else {
                return APP_QUIT;
            }
            focus = 0;
            break;
        default: {
            int action = update_page(&m->pages[m->window], &focus, code);
            switch (action) {
                case QUIT:
                    return APP_QUIT;
                case BACK:
                    if (m->window == MENU_CONFIG || m->window == MENU_EMITTERS) {
                        m->window = MENU_SETTINGS;
                    } else if (m->window == MENU_SETTINGS) {
                        m->window = MENU_MAIN;
                    }
                    focus = 0;
                    break;
                case RUN:
                    m->window = MENU_NONE;
                    focus = 0;
                    return APP_SIM;
                    break;
                case SETTINGS:
                    m->window = MENU_SETTINGS;
                    focus = 0;
                    break;
                case CONFIG:
                    m->window = MENU_CONFIG;
                    focus = 0;
                    break;
                case EMITTERS:
                    m->window = MENU_EMITTERS;
                    focus = 0;
                    break;
                case WIDGET_NONE:
                default: break;
            }
        }
    }

    return APP_MENU;
}

void draw_menu(const menu* m, ivec2 size) {
    clear_screen();
    if (m->window == MENU_NONE) return;
    draw_page(&m->pages[m->window], focus, size);
    print_framebuffer();
}