//
// Created by aaa on 16/10/2026.
//

#include "smoke.h"
#include "smoke_physics.h"
#include "smoke_graphics.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

extern environment smoke;

void set_color(int r, int g, int b) {

    if (r < 0) r = 0;
    if (g < 0) g = 0;
    if (b < 0) b = 0;

    if ( r > 255) r = 255;
    if ( g > 255) g = 255;
    if ( b > 255) b = 255;

    //printf("\033[38;%d;%d;%d;0m");
    putchar(27);
    putchar('['), putchar('3'), putchar('8');
    putchar(';'), putchar('2');
    putchar(';');

    if (r >= 100) putchar('0' + r / 100);
    if (r >= 10) putchar('0' + (r/10) % 10);
    putchar('0' + r % 10);

    putchar(';');

    if (g >= 100) putchar('0' + g / 100);
    if (g >= 10) putchar('0' + (g/10) % 10);
    putchar('0' + g % 10);

    putchar(';');

    if (b >= 100) putchar('0' + b / 100);
    if (b >= 10) putchar('0' + (b/10) % 10);
    putchar('0' + b % 10);

    putchar('m');
}

void reset_color() {
    putchar(27), putchar('['), putchar('0'), putchar('m');
}

void print_smoke() {
    const char levels[] = " .:-=+*#%@";
    int n_levels = sizeof(levels) -1;

    int to_init = 1;
    float prev_r, prev_g, prev_b;
    const float EPSILON = 1.0f;

    printf("\x1b[H");
    for (int y = smoke.size.y - 1; y >= 0; y--) {
        for (int x = 0; x < smoke.size.x; ++x) {

            if (is_solid((ivec2){x,y})) {reset_color(); putchar('&'); continue;}

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
                prev_r = r;
                prev_g = g;
                prev_b = b;
                to_init = 0;
            }

            if (fabsf(r - prev_r) > EPSILON ||
                fabsf(g - prev_g) > EPSILON ||
                fabsf(b - prev_b) > EPSILON)
                set_color((int)r,(int)g,(int)b);

            putchar(levels[level_idx]);

            prev_r = r;
            prev_g = g;
            prev_b = b;
        }
        putchar('\n');
    }
    reset_color();
}