//
// Created by aaa on 28/10/2026.
//

#include <time.h>
#include <unistd.h>

#include "app.h"

#include "menu.h"
#include "graphics.h"
#include "terminal.h"

static struct timespec start_time, end_time;

void app_init(app* a, const smoke_config config) {
    //Init physics related info
    a->state = APP_MENU;

    a->size = detect_size();

    a->config = config;
    a->sim = init_simulation(config, a->size);

    init_menu(&a->menu, &a->config);
    init_graphics();

    //Setup terminal
    set_handlers();
    register_app(a);
    setup_terminal();
    set_atexit();
}

static void frame_start() {
    clock_gettime(CLOCK_MONOTONIC, &start_time);
}

static void frame_limit(const smoke_config c) {
    const long target_frame_time_us = (long)(1000000.0f / (float)c.target_fps);
    /* why the hell do you keep warning me on a redundant long
     * cast if you warn me on implicit cast if I remove it >:/
     */

    clock_gettime(CLOCK_MONOTONIC, &end_time);

    const long elapsed_us = (end_time.tv_sec - start_time.tv_sec) * 1000000L + (end_time.tv_nsec - start_time.tv_nsec) / 1000L;
    if (elapsed_us < target_frame_time_us) {
        usleep(target_frame_time_us - elapsed_us);
    }
}

static void handle_resize(app *a) {
    if (should_resize()) {
        a->size = detect_size();
        restart_simulation(&a->sim, a->size);
        init_graphics();
    }
}

void app_run(app* a) {
    while (should_keep_running() && a->state != APP_QUIT) {
        handle_resize(a);
        int in = parse_input();

        frame_start();

        switch (a->state) {
            case APP_MENU: {
                int next = update_menu(in);
                if (next == APP_SIM) {
                    free_simulation(&a->sim);
                    a->sim = init_simulation(a->config, a->size);
                    init_graphics();
                }
                a->state = next;
                if (a->state == APP_MENU) draw_menu(a->size);
                break;
            }
            case APP_SIM:
                if (in == ACT_QUIT) { a->state = APP_MENU; break; }
                handle_obstacle_movement(&a->sim.obstacle, in, a->sim.env.size);
                run(&a->sim);
                print_smoke(&a->sim.env);
                break;
            default: break;
        }

        frame_limit(a->config);
    }
}

void app_shutdown(app* a) {
    clean_screen();
    free_simulation(&a->sim);
    reset_terminal();
}
