//
// Created by aaa on 22/10/2026.
//

#include "obstacles.h"

static void clamp_obstacle(obstacle *o, const ivec2 max_size) {
    
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