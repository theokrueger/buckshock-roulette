cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(buckshock-roulette)
pico_sdk_init()

add_executable(buckshock-roulette
    src/debug.c
    src/output.c
    src/input.c
    src/game.c
    src/main.c
)

pico_enable_stdio_usb(buckshock-roulette 1)
pico_enable_stdio_uart(buckshock-roulette 0)
target_link_libraries(buckshock-roulette pico_stdlib hardware_pwm pico_time pico_sync pico_multicore pico_rand pico_stdio)
pico_add_extra_outputs(buckshock-roulette)
