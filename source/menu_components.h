//
// Created by aaa on 28/10/2026.
//

#ifndef SMOKESSH_MENU_COMPONENTS_H
#define SMOKESSH_MENU_COMPONENTS_H

#include "sim_math.h"

typedef enum {W_BUTTON, W_INT, W_FLOAT} widget_type;

typedef struct widget {
    widget_type type;

    const char* label;

    float min, max, step;
    int id;

    void (*callback )(int caller_id);
    void* data;
} widget;

#define NO_ID (-1)
#define MAX_WIDGETS_PAGE 16

typedef struct page {
    const char* title;

    widget items[MAX_WIDGETS_PAGE];
    int w_count;

    int focus;
} page;

widget create_button(const char* label, int id,void (*callback)(int caller_id));
widget create_input_i(const char* label, int* data, int min, int max, int step);
widget create_input_f(const char* label, float* data, float min, float max, float step);

int widget_input(widget* w, int act);
void draw_widget(const widget* w, int col, int row, int color, int flags, char wrap_start, char wrap_end);

void init_page(page* p);
void add_widget(page* p, widget w);
int update_page(page* p, int act);
void draw_page(const page* p, ivec2 size);

#endif //SMOKESSH_MENU_COMPONENTS_H
