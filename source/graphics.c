//
// Created by aaa on 16/10/2026.
//

#include "smoke.h"
#include "physics.h"
#include "graphics.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>
#include <poll.h>

extern environment smoke;

static char frame_buffer[BUFFER_SIZE];
static char drawn_ch[MAX_ANY_SIZE][MAX_ANY_SIZE];
static int drawn_col[MAX_ANY_SIZE][MAX_ANY_SIZE];

static int cur_row = -1;
static int cur_col = -1;
static int cur_color = -1;

static int buf_len = 0;

static void push_char(const char c) {
    if (buf_len < BUFFER_SIZE -1) {
        frame_buffer[buf_len++] = c;
    }
}

static void push_str(const char* str) {
    while (*str && buf_len < BUFFER_SIZE - 1) {
        frame_buffer[buf_len++] = *str++;
    }
}

static void push_int(const int n) {
    if (n >= 100) push_char((char)((int)'0' + n / 100));
    if (n >= 10) push_char((char)((int)'0' + (n/10) % 10));
    push_char((char)((int)'0' + n % 10));
}

static void move_cursor(int r, int c) {
    push_str("\033["), push_int(r), push_char(';'), push_int(c), push_char('H');
}

void clear_screen() {
    push_str("\033[2J");
}

static void hide_cursor() {
    push_str("\033[?25l");
}

void show_cursor() {
    push_str("\033[?25h");
}

static void set_background(int index) {
    push_str("\033[48;5;");
    push_int(index);
    push_char('m');
}

static void reset_color() {
    push_str("\033[0m");
}

static int get_closest_color_index(int r, int g, int b) {
    r = (5 * r) / 255;
    g = (5 * g) / 255;
    b = (5 * b) / 255;
    return 16 + 36*r + 6 * g + b;
}

static void set_color_rgb(int r, int g, int b) {

    if (r < 0) r = 0; if ( r > 255) r = 255;
    if (g < 0) g = 0; if ( g > 255) g = 255;
    if (b < 0) b = 0; if ( b > 255) b = 255;

    push_str("\033[38;5;");

    push_int(get_closest_color_index(r,g,b));

    push_char('m');
}

static void set_color_index(int index) {
    push_str("\033[38;5;");
    push_int(index);
    push_char('m');
}

static void flush_buffer() {
    int off = 0;
    while (off < buf_len) {
        const ssize_t n = write(STDOUT_FILENO, frame_buffer + off, buf_len - off);
        if (n > 0) {
            off += (int)n;
        } else if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            struct pollfd p = {.fd = STDOUT_FILENO, .events = POLLOUT};
            poll(&p, 1, 100);
        } else if (n < 0 && errno == EINTR) {
            continue;
        } else {
            break;
        }
    }

    buf_len = 0;
}

static void print_framebuffer() {
    flush_buffer();
    fflush(stdout);
    buf_len = 0;
}

static void set_bold() {
    push_str("\033[1m");
}

void init_graphics() {
    for (int y = 0; y < MAX_ANY_SIZE; ++y) {
        for (int x = 0; x < MAX_ANY_SIZE; ++x) {
            drawn_col[y][x] = -1;
            drawn_ch[y][x] = 0;
        }
    }
    clear_screen();
    hide_cursor();
}

static void print_header() {
    const char *title = " q to quit";

    int title_len = 10;

    move_cursor(1,1);
    set_background(get_closest_color_index(0,250,250));
    set_color_rgb(0,0,0);
    set_bold();

    push_str(title);
    for (int x = title_len; x < smoke.size.x; ++x) {
        push_char(' ');
    }
    reset_color();
}

void print_smoke() {
    cur_row = cur_col = cur_color = -1;

    const char levels[] = " .:-=+*#%@";
    int n_levels = sizeof(levels) -1;

    print_header();

    for (int sy = 1; sy < smoke.size.y; ++sy) {
        int y = smoke.size.y - 1 - sy;

        for (int x = 0; x < smoke.size.x; ++x) {
            int col;
            char ch;

            if (is_solid((ivec2){x,y})) {
                col = 244;
                ch = '&';
            } else {
                float r = smoke.smoke[smoke.s_idx][x][y].x;
                float g = smoke.smoke[smoke.s_idx][x][y].y;
                float b = smoke.smoke[smoke.s_idx][x][y].z;

                int level_idx = (int)(r+g+b);
                if (level_idx < 0) level_idx = 0;
                if (level_idx >= n_levels) level_idx = n_levels -1;
                ch = levels[level_idx];

                const float max = fmaxf(r, fmaxf(g, b));
                if (max != 0) {
                    r = 255.0f * (r / max);
                    g = 255.0f * (g / max);
                    b = 255.0f * (b / max);
                }
                col = get_closest_color_index((int) r, (int) g, (int) b);

                if (ch == ' ' && drawn_col[sy][x] >= 0) col = drawn_col[sy][x];
            }

            if (ch == drawn_ch[sy][x] && col == drawn_col[sy][x]) { continue; }

            if (cur_row != sy || cur_col != x) {move_cursor(sy + 1, x + 1);}

            if (ch != ' ' && col != cur_color) {
                set_color_index(col); cur_color = col;
            }

            push_char(ch);

            cur_row = sy; cur_col = x+1;

            if (x == smoke.size.x - 1) cur_row = -1;

            drawn_ch[sy][x] = ch; drawn_col[sy][x] = col;
        }
    }
    print_framebuffer();
}