# 대학교 시간표 자동 생성기 — CMake 빌드 파일
#
# 사용법:
#   - Visual Studio 2026: "파일 → 열기 → 폴더"로 이 폴더를 열면 자동 구성됩니다.
#   - 명령줄(윈도우/맥/리눅스):
#       cmake -B build
#       cmake --build build
#     결과 실행 파일은 build/ (윈도우는 build/Debug 또는 build/Release) 안에 생깁니다.

cmake_minimum_required(VERSION 3.15)
project(schedule LANGUAGES C)

# C11 표준으로 컴파일합니다.
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
    # /utf-8         : 한글이 든 소스와 문자열을 UTF-8로 읽고 출력합니다.
    # _CRT_SECURE_NO_WARNINGS : fopen·strtok 등 표준 함수의 MSVC 경고(C4996)를 끕니다.
    target_compile_options(schedule PRIVATE /utf-8 /W3)
    target_compile_definitions(schedule PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
    # gcc·clang(맥/리눅스)
    target_compile_options(schedule PRIVATE -Wall -Wextra)
endif()
