//
// Created by aaa on 16/10/2026.
//

#include "smoke.h"
#include "smoke_physics.h"
#include "smoke_graphics.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

extern environment smoke;

char frame_buffer[BUFFER_SIZE];

int buf_len = 0;

static void push_char(char c) {
    if (buf_len < BUFFER_SIZE -1) {
        frame_buffer[buf_len++] = c;
    }
}

static void push_str(const char* str) {
    while (*str && buf_len < BUFFER_SIZE - 1) {
        frame_buffer[buf_len++] = *str++;
    }
}

static int get_closest_color_index(int r, int g, int b) {
    r = (5 * r) / 255;
    g = (5 * g) / 255;
    b = (5 * b) / 255;
    return 16 + 36*r + 6 * g + b;
}

static void set_color(int r, int g, int b) {

    if (r < 0) r = 0; if ( r > 255) r = 255;
    if (g < 0) g = 0; if ( g > 255) g = 255;
    if (b < 0) b = 0; if ( b > 255) b = 255;

    //push_str("\033[38;2;");
    push_str("\033[38;5;");

    int index = get_closest_color_index(r,g,b);

    if (index >= 100) push_char('0' + index / 100);
    if (index >= 10) push_char('0' + (index/10) % 10);
    push_char('0' + index % 10);
    push_char('m');
}

static void reset_color() {
    push_str("\033[0m");
}

static void reset_pointer() {
    push_str("\033[H");
}

static void print_framebuffer() {
    write(STDOUT_FILENO, frame_buffer, buf_len);
    fflush(stdout);
    buf_len = 0;
}

void print_smoke() {
    const char levels[] = " .:-=+*#%@";
    int n_levels = sizeof(levels) -1;

    int to_init = 1;
    float prev_r, prev_g, prev_b;
    const float EPSILON = 1.0f;

    reset_pointer();

    for (int y = smoke.size.y - 1; y >= 0; y--) {
        for (int x = 0; x < smoke.size.x; ++x) {
            if (is_solid((ivec2){x,y})) {reset_color(); push_char('&'); continue;}

            float r = smoke.smoke[smoke.s_idx][x][y].x;
            float g = smoke.smoke[smoke.s_idx][x][y].y;
            float b = smoke.smoke[smoke.s_idx][x][y].z;

            int level_idx = (int)(r+g+b);
            if (level_idx < 0) level_idx = 0;
            if (level_idx >= n_levels) level_idx = n_levels -1;

            float max = fmaxf(r, fmaxf(g, b));

            if (max != 0) {
                r = 255.0f * (r / max);
                g = 255.0f * (g / max);
                b = 255.0f * (b / max);
            }

            if (to_init) {
                to_init = 0;
                set_color((int)r,(int)g,(int)b);
            } else if (fabsf(r - prev_r) >= EPSILON ||
                        fabsf(g - prev_g) >= EPSILON ||
                        fabsf(b - prev_b) >= EPSILON) {
                set_color((int)r,(int)g,(int)b);
            }

            push_char(levels[level_idx]);

            prev_r = r; prev_g = g; prev_b = b;
        }
        push_char('\n');
    }

    print_framebuffer();
}