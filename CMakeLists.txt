cmake_minimum_required(VERSION 3.15)
project(schedule LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(schedule
    main.c
    course.c
    schedule.c
    optimizer.c
    display.c
    export.c
)

if(MSVC)
    target_compile_options(schedule PRIVATE /utf-8 /W3)
    target_compile_definitions(schedule PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
    target_compile_options(schedule PRIVATE -Wall -Wextra)
endif()
