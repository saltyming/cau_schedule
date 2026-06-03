# 빌드 구조

`Makefile`은 여섯 개의 C 파일을 각각 오브젝트 파일로 컴파일한 뒤, 최종 실행 파일 `schedule`로 링크합니다.

```mermaid
flowchart TD
  A["make"] --> B["all"]
  B --> C["schedule"]

  D["main.c"] --> D1["main.o"]
  E["course.c"] --> E1["course.o"]
  F["schedule.c"] --> F1["schedule.o"]
  G["optimizer.c"] --> G1["optimizer.o"]
  H["display.c"] --> H1["display.o"]
  I["export.c"] --> I1["export.o"]

  D1 --> C
  E1 --> C
  F1 --> C
  G1 --> C
  H1 --> C
  I1 --> C

  C --> J["./schedule"]
```

## 컴파일 설정

```mermaid
flowchart LR
  CC["CC = gcc"]
  CFLAGS["CFLAGS = -Wall -Wextra -std=c11 -O2"]
  SRCS["SRCS = main.c course.c schedule.c optimizer.c display.c export.c"]
  OBJS["OBJS = 각 .c의 .o"]
  TARGET["TARGET = schedule"]

  CC --> TARGET
  CFLAGS --> TARGET
  SRCS --> OBJS
  OBJS --> TARGET
```

## `clean` 타깃

`make clean`은 빌드 산출물(오브젝트 파일과 실행 파일)을 모두 지웁니다.

```mermaid
flowchart LR
  CL["make clean"] --> RM["rm -f $(OBJS) $(TARGET)"]
  RM --> R1["*.o 삭제"]
  RM --> R2["schedule 삭제"]
```

## CMake 빌드 (Visual Studio · 크로스 플랫폼)

`Makefile`과 별개로 `CMakeLists.txt`도 같은 여섯 개 소스를 빌드합니다. Visual Studio 2026은
"파일 → 열기 → 폴더"로 자동 구성되고, 명령줄에서는 아래 두 명령으로 빌드합니다.

```mermaid
flowchart TD
  A["cmake -B build"] --> B["build/ 빌드 구성 생성"]
  B --> C["cmake --build build"]
  C --> D["schedule 실행 파일<br/>(윈도우: build/Debug 또는 build/Release)"]

  S["main.c · course.c · schedule.c<br/>optimizer.c · display.c · export.c"] --> C
  STD["C11 (CMAKE_C_STANDARD)"] --> C
  MSVC["MSVC: /utf-8 /W3<br/>+ _CRT_SECURE_NO_WARNINGS"] --> C
  GCC["gcc · clang: -Wall -Wextra"] --> C
```

> 컴파일 플래그 차이: `Makefile`은 `-O2` 최적화를 켜지만, `CMakeLists.txt`에는 별도
> 최적화 플래그가 없어 빌드 타입 기본값을 따릅니다.
