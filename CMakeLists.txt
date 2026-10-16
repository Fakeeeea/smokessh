cmake_minimum_required(VERSION 3.12)

project(smokessh LANGUAGES C)

add_executable(smokessh source/smoke.c source/smoke.h
        source/terminal.c
        source/terminal.h
        source/smoke_math.c
        source/smoke_math.h)

target_link_libraries(smokessh PRIVATE m)