//
// Created by aaa on 15/10/2026.
//

#ifndef SMOKESSH_SMOKE_MATH_H
#define SMOKESSH_SMOKE_MATH_H

typedef struct ivec2 {
    int x,y;
}ivec2;

typedef struct fvec2 {
    float x,y;
}fvec2;

typedef struct fvec3 {
    float x,y,z;
}fvec3;

float sample_bilinear(float** data, int width, int height, float x, float y);
float lerp(float a, float b, float t);

#endif //SMOKESSH_SMOKE_MATH_H
