//
// Created by aaa on 28/10/2026.
//

#include <stdlib.h>
#include <stdio.h>

#include "terminal.h"

#include "menu_components.h"
#include "graphics.h"

widget create_button(char* label, int id, void (*callback)(int caller_id)) {
    return (widget){
        .label = label,

        .type = W_BUTTON,

        .max = 0, .min = 0, .step = 0,
        .id = id,

        .data = NULL,
        .callback = callback,
    };
}

widget create_input_i(char* label, int* data, int min, int max, int step) {
    return (widget){
        .label = label,

        .type = W_INT,

        .max = (float)max, .min = (float)min, .step = (float)step,
        .id = NO_ID,

        .data = data,
        .callback = NULL,
    };
}
widget create_input_f(char* label, float* data, float min, float max, float step) {
    return (widget){
        .label = label,

        .type = W_FLOAT,

        .max = max, .min = min, .step = step,
        .id = NO_ID,

        .data = data,
        .callback = NULL,
    };
}

static void increment_widget(widget *w) {
    if (w->type != W_INT && w->type != W_FLOAT) return;
    //i hate this
    switch (w->type) {
        case W_INT:
            *(int*)w->data = *(int*)w->data + (int) w->step;
            if (*(int*)w->data > (int)w->max) *(int*)w->data = (int)w->max;
            break;
        case W_FLOAT:
            *(float*)w->data = *(float*)w->data + w->step;
            if (*(float*)w->data > w->max) *(float*)w->data = w->max;
            break;
        default: break;
    }
}
static void decrement_widget(widget *w) {
    if (w->type != W_INT && w->type != W_FLOAT) return;
    //i hate this too by the way
    switch (w->type) {
        case W_INT:
            *(int*)w->data = *(int*)w->data - (int) w->step;
            if (*(int*)w->data < (int)w->min) *(int*)w->data = (int)w->min;
            break;
        case W_FLOAT:
            *(float*)w->data = *(float*)w->data - w->step;
            if (*(float*)w->data < w->min) *(float*)w->data = w->min;
            break;
        default: break;
    }
}

int widget_input(widget* w, int act) {
    if (!w) return NO_ID;

    switch (act) {
        case ACT_RIGHT:
            increment_widget(w);
            break;
        case ACT_LEFT:
            decrement_widget(w);
            break;
        case ACT_SELECT:
            if (w->type == W_BUTTON) { w->callback(w->id); }
            break;
        default: break;
    }

    return 1;
}

void draw_widget(const widget* w, int col, int row, int focused) {
    char val[64] = "";
    char buf[256];

    switch (w->type) {
        case W_INT:   snprintf(val, sizeof val, " - %d",   *(int*)w->data);   break;
        case W_FLOAT: snprintf(val, sizeof val, " - %.3f", *(float*)w->data); break;
        default: break;
    }
    snprintf(buf, sizeof buf, "%c%s%s%c",
             focused ? '[' : ' ', w->label, val, focused ? ']' : ' ');

    draw_text(col, row, buf, 255, 0, 0);
}

void add_widget(page* p, widget w) {
    if (p->w_count >= MAX_WIDGETS_PAGE) {
        fprintf(stderr, "Tried adding a widget to an already full page (%s).", p->title);
        exit(EXIT_FAILURE);
    }

    p->w_count++;
    p->items[p->w_count-1] = w;
}

void init_page(page* p) {
    p->w_count = 0;
    p->focus = 0;
    p->title = "";
}

int update_page(page* p, int act) {
    if (p == NULL) return 1;

    if (act == ACT_UP)   p->focus = (p->focus + p->w_count - 1) % p->w_count;
    if (act == ACT_DOWN) p->focus = (p->focus + 1) % p->w_count;
    return widget_input(&p->items[p->focus], act);
}
void draw_page(const page* p, ivec2 size) {
    if (p == NULL) return;

    const int w = 30;
    const int total_h = p->w_count * 2 + 2;
    const int top  = (size.y - total_h) / 2 + 1;
    const int left = (size.x - w) / 2 + 1;

    draw_text(left, top, p->title, 255, 0, 1);
    for (int i = 0; i < p->w_count; ++i)
        draw_widget(&p->items[i], left, top + 2 + i * 2, i == p->focus);
}