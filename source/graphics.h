//
// Created by aaa on 16/10/2026.
//

#ifndef SMOKESSH_SMOKE_GRAPHICS_H
#define SMOKESSH_SMOKE_GRAPHICS_H

#include "simulation.h"

/* max grid area = 16.384 pixels.
/* Around 64 bytes per pixel (prepared for the worst!!)
 */

#define BUFFER_SIZE (1024 * 1024)

#define NONE 0
#define BOLD 0x1
#define ITALIC 0x2
#define UNDERLINE 0x4
#define STRIKETHROUGH 0x8
#define FAINT 0x16
#define BLINK 0x20


void init_graphics();
void print_smoke(const environment* smoke);
void print_framebuffer();

void clear_screen();
void clean_screen();

int get_closest_color_index(int r, int g, int b);
void draw_text(int col, int row, const char* str, int fg, int bg, int flags);

#endif //SMOKESSH_SMOKE_GRAPHICS_H
