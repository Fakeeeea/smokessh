cmake_minimum_required(VERSION 3.12)

project(smokessh LANGUAGES C)

add_executable(smokessh source/smoke.c source/smoke.h
        source/terminal.c
        source/terminal.h
        source/smoke_math.c
        source/smoke_math.h
        source/physics.c
        source/physics.h
        source/graphics.c
        source/graphics.h
        source/parser.c
        source/parser.h
        source/utils.h
        source/main.c
        source/config.h)

target_link_libraries(smokessh PRIVATE m)