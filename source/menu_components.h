//
// Created by aaa on 28/10/2026.
//

#ifndef SMOKESSH_MENU_COMPONENTS_H
#define SMOKESSH_MENU_COMPONENTS_H

#include "sim_math.h"

typedef enum {W_BUTTON, W_TOGGLE, W_INT, W_FLOAT} widget_type;

typedef struct widget {
    widget_type type;
    const char* label;
    void* data;
    float min, max, step;
    int id;
} widget;

#define WIDGET_NONE (-1)

typedef struct page {
    const char* title;
    widget* items;
    int count;
} page;

widget create_button(char* label, int id);
widget create_input_i(char* label, int* data, int min, int max, int step);
widget create_input_f(char* label, float* data, float min, float max, float step);

int widget_input(widget* w, int act);
void draw_widget(widget* w, int col, int row, int width, int focused);

int update_page(page* p, int* focus, int act);
void draw_page(const page* p, int focus, ivec2 size);

#endif //SMOKESSH_MENU_COMPONENTS_H
