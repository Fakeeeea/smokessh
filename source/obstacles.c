//
// Created by aaa on 22/10/2026.
//

#include "obstacles.h"
#include "terminal.h"

static void clamp_obstacle(obstacle *o, const ivec2 max_size) {
    if (o->location.x < 0) o->location.x = 0;
    if (o->location.y < 0) o->location.y = 0;
    if (o->location.x >= max_size.x) o->location.x = max_size.x-1;
    if (o->location.y >= max_size.y) o->location.y = max_size.y-1;
}

obstacle init_obstacle(const ivec2 size, const int radius) {
    return (obstacle) {
        .location.x = (int)((float)size.x * 0.5f),
        .location.y = (int)((float)size.y * 0.5f),
        .radius = radius
    };
}

void move_obstacle(obstacle *o, const ivec2 new_pos, const ivec2 size) {
    o->location.x = new_pos.x;
    o->location.y = new_pos.y;
    clamp_obstacle(o, size);
}

void handle_obstacle_movement(obstacle *o, int code, const ivec2 size) {
    ivec2 mov = {0,0};
    switch (code) {
        case ACT_RIGHT: mov.x++; break;
        case ACT_LEFT: mov.x--; break;
        case ACT_UP: mov.y++; break;
        case ACT_DOWN: mov.y--; break;
        default:  break;
    }

    ivec2 new_pos = {o->location.x + mov.x, o->location.y + mov.y};

    move_obstacle(o, new_pos, size);
}

