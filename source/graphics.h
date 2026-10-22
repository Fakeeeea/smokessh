//
// Created by aaa on 16/10/2026.
//

#ifndef SMOKESSH_SMOKE_GRAPHICS_H
#define SMOKESSH_SMOKE_GRAPHICS_H

// max grid area = 16.384 pixels.
// Around 64 bytes per pixel (prepared for the worst!!)
#define BUFFER_SIZE (1024 * 1024)

void init_graphics();
void print_smoke(const environment* smoke);
void clear_screen();
void show_cursor();

#endif //SMOKESSH_SMOKE_GRAPHICS_H
