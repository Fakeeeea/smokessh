//
// Created by aaa on 15/10/2026.
//

#include "smoke_math.h"
#include <math.h>

float sample_bilinear(float* data, const int width, const int height, const float x, const float y) {
    int x0 = (int) floorf(x);
    int y0 = (int) floorf(y);

    int x1 = x0 +1;
    int y1 = y0 +1;

    if (x0 < 0) x0 = 0; if (x0 >= width) x0 = width-1;
    if (y0 < 0) y0 = 0; if (y0 >= height) y0 = height-1;
    if (x1 >= width) x1 = width-1; if (x1 < 0) x1 = 0;
    if (y1 >= height) y1 = height-1; if (y1 < 0) y1 = 0;

    const float tx = x - (float)x0;
    const float ty = y - (float)y0;

#define D(i,j) (data[j * width + i])
    float a = lerp(D(x0,y0), D(x1,y0), tx);
    float b = lerp(D(x0,y1), D(x1,y1), tx);
#undef D

    return lerp(a, b, ty);
}

fvec3 sample_bilinear_fvec3(fvec3* data, int width, int height, float x, float y) {
    int x0 = (int) floorf(x);
    int y0 = (int) floorf(y);

    int x1 = x0 +1;
    int y1 = y0 +1;

    if (x0 < 0) x0 = 0; if (x0 >= width) x0 = width-1;
    if (y0 < 0) y0 = 0; if (y0 >= height) y0 = height-1;
    if (x1 >= width) x1 = width-1; if (x1 < 0) x1 = 0;
    if (y1 >= height) y1 = height-1; if (y1 < 0) y1 = 0;

    const float tx = x - (float) x0;
    const float ty = y - (float) y0;

    fvec3 a, b;

#define D(i,j) (data[j * width + i])
    a.x = lerp(D(x0,y0).x, D(x1,y0).x, tx);
    b.x = lerp(D(x0,y1).x, D(x1,y1).x, tx);

    a.y = lerp(D(x0,y0).y, D(x1,y0).y, tx);
    b.y = lerp(D(x0,y1).y, D(x1,y1).y, tx);

    a.z = lerp(D(x0,y0).z, D(x1,y0).z, tx);
    b.z = lerp(D(x0,y1).z, D(x1,y1).z, tx);
#undef D

    return (fvec3) {
        lerp(a.x, b.x, ty),
        lerp(a.y, b.y, ty),
        lerp(a.z, b.z, ty)
    };
}

float lerp(float a, float b, float t) {
    return a + t * (b-a);
}