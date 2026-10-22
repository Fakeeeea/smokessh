cmake_minimum_required(VERSION 3.12)

project(smokessh LANGUAGES C)

add_executable(smokessh source/simulation.c source/simulation.h
        source/terminal.c
        source/terminal.h
        source/sim_math.c
        source/sim_math.h
        source/physics.c
        source/physics.h
        source/graphics.c
        source/graphics.h
        source/utils.h
        source/main.c
        source/config.h
        source/emitters.c
        source/emitters.h
        source/obstacles.c
        source/obstacles.h)

target_link_libraries(smokessh PRIVATE m)