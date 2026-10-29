//
// Created by aaa on 22/10/2026.
//

#ifndef SMOKESSH_OBSTACLES_H
#define SMOKESSH_OBSTACLES_H

#define INIT_RADIUS 2

#include "sim_math.h"

typedef struct obstacle {
    ivec2 location;
    int radius;
}obstacle;

obstacle init_obstacle(ivec2 size, int radius);
void move_obstacle(obstacle *o, ivec2 new_pos, ivec2 size);
void handle_obstacle_movement(obstacle *o, int code, ivec2 size);

#endif //SMOKESSH_OBSTACLES_H
