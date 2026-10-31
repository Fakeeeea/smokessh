//
// Created by aaa on 28/10/2026.
//

#include <stdlib.h>
#include <stdio.h>

#include "menu.h"
#include "app.h"
#include "graphics.h"
#include "terminal.h"

#include <dirent.h>
#include <string.h>
#include <sys/stat.h>

static menu* m;

typedef enum menu_ids {RUN, SETTINGS, QUIT, BACK, CONFIG, EMITTERS} menu_ids;

static void quit_app(const int id) {
    (void) id;
    m->result = APP_QUIT;
}

static void start_sim(const int id) {
    (void) id;
    m->result = APP_SIM;
}

static void push_stack(const int val) {
    if (m->depth >= MAX_DEPTH) {
        fprintf(stderr, "Tried pushing over menu stack limit");
        exit(EXIT_FAILURE);
    }

    m->depth++;
    m->stack[m->depth-1] = val;
}

static void pop_stack() {
    if (m->depth <= 0) {
        fprintf(stderr, "Tried popping an empty menu stack");
        exit(EXIT_FAILURE);
    }
    m->depth--;
}

static void prev_page(const int id) {
    (void)id;
    pop_stack();
}

static int qsort_cmp(const void *a, const void *b) {return strcmp(a,b);}

static void scan_presets() {
    m->preset_count = 0;

    DIR* d = opendir(m->config->emitter_dir);
    if (!d) return;

    struct dirent* e;
    while ((e = readdir(d)) && m->preset_count < MAX_PRESETS) {
        if (e->d_name[0] == '.') continue;
        if (strlen(e->d_name) >= PRESET_NAME_LEN) continue;

        char path[PATH_MAX];
        snprintf(path, sizeof(path), "%s/%s", m->config->emitter_dir, e->d_name);

        struct stat st;
        if (stat(path, &st) != 0 || !S_ISREG(st.st_mode)) continue;

        strcpy(m->presets[m->preset_count++], e->d_name);
    }
    closedir(d);

    qsort(m->presets, (size_t)m->preset_count, PRESET_NAME_LEN, qsort_cmp);
}

static void select_preset(int id) {
    snprintf(m->selected, sizeof(m->selected), "%s/%s", m->config->emitter_dir, m->presets[id]);
    m->config->emitter_file = m->selected;
    prev_page(0);
}

static void build_emitters_page() {
    page* p = &m->pages[PAGE_EMITTERS];
    init_page(p);
    scan_presets();

    p->title = m->preset_count ? "Emitter presets" : "No presets found";
    for (int i = 0; i < m->preset_count; ++i) {
        add_widget(p, create_button(m->presets[i], i, select_preset));
    }
    add_widget(p, create_button("Back", BACK, prev_page));
}

static void next_page(const int id) {
    switch (id) {
        case SETTINGS:
            push_stack(PAGE_SETTINGS);
            break;
        case EMITTERS:
            build_emitters_page();
            push_stack(PAGE_EMITTERS);
            break;
        case CONFIG:
            push_stack(PAGE_CONFIG);
            break;
        default: break;
    }
}

void init_menu(menu* i_menu, smoke_config* config) {
    m = i_menu;

    m->config = config;
    m->depth = 1;
    m->stack[i_menu->depth-1] = PAGE_MAIN;

#define P(id) (m->pages[id])

    for (int i = 0; i < PAGE_COUNT; ++i) { init_page(&P(i)); }

    P(PAGE_MAIN).title = "./smokessh\0";

    add_widget(&P(PAGE_MAIN), create_button("Run", RUN, start_sim));
    add_widget(&P(PAGE_MAIN),create_button("Settings", SETTINGS, next_page));
    add_widget(&P(PAGE_MAIN),create_button("Quit", QUIT, quit_app));

    P(PAGE_SETTINGS).title = "Settings\0";
    add_widget(&P(PAGE_SETTINGS), create_button("Config", CONFIG, next_page));
    add_widget(&P(PAGE_SETTINGS), create_button("Emitters preset", EMITTERS, next_page));
    add_widget(&P(PAGE_SETTINGS), create_button("Back", BACK, prev_page));

    P(PAGE_CONFIG).title = "Config A/D - ARROWS\0";
    add_widget(&P(PAGE_CONFIG), create_input_i("T Ambient", &config->t_ambient, -9999, 9999, 1));
    add_widget(&P(PAGE_CONFIG), create_input_i("Obstacle radius", &config->obstacle_radius, 0, 9999, 1));
    add_widget(&P(PAGE_CONFIG), create_input_f("Cell size", &config->cell_size, 0.01f, 1, 0.01f));
    add_widget(&P(PAGE_CONFIG), create_input_f("Time step", &config->time_step, 0.001f, 1, 0.001f));
    add_widget(&P(PAGE_CONFIG), create_input_f("Wind", &config->wind, -10, 10, 1));
    add_widget(&P(PAGE_CONFIG), create_button("Back", BACK, prev_page));
#undef P

}

static page* get_current_page() {
    if (m->depth == 0) return NULL;
    return &m->pages[m->stack[m->depth-1]];
}

int update_menu(const int code) {
    m->result = APP_MENU;

    if (code == ACT_QUIT) {
        prev_page(0);
        if (m->depth == 0) m->result = APP_QUIT;
    } else {
        update_page(get_current_page(), code);
    }

    return m->result;
}

void draw_menu(const ivec2 size) {
    clear_screen();
    if (m->depth == 0) return;

    draw_page(get_current_page(), size);
    print_framebuffer();
}