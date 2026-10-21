//
// Created by aaa on 19/10/2026.
//

#ifndef SMOKESSH_SMOKE_UTILS_H
#define SMOKESSH_SMOKE_UTILS_H

#define MALLOC(ptr, size)   \
do {                        \
    (ptr) = malloc(size);   \
    if ((ptr) == NULL) {    \
        perror("malloc");   \
        exit(EXIT_FAILURE); \
    }                       \
}while (0)                  \

#define IDX(x,y,w) ((size_t)(y) * (w) + (x))

#endif //SMOKESSH_SMOKE_UTILS_H
