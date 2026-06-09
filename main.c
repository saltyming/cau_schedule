// 화면 출력·입력(printf, fopen 등)을 위한 표준 입출력 라이브러리
#include <stdio.h>
// 강의 입력·CSV 읽기·목록 출력 함수가 선언된 헤더
#include "course.h"
// 시간표 검사 함수와 Schedule 구조체가 선언된 헤더
#include "schedule.h"
// 시간표 자동 생성(최적화) 함수가 선언된 헤더
#include "optimizer.h"
// 시간표를 화면에 보여 주는 함수가 선언된 헤더
#include "display.h"
// 시간표를 CSV로 저장하는 함수가 선언된 헤더
#include "export.h"

/*
 * main.c
 * ------
 * 프로그램의 시작점입니다. 사용자가 메뉴에서 무엇을 선택했는지에 따라
 * 강의를 입력하거나, CSV를 불러오거나, 최적 시간표를 생성하는 함수를 호출합니다.
 */

// 아래 블록은 윈도우(_WIN32)에서 컴파일할 때만 포함된다
#ifdef _WIN32
// windows.h 에서 잘 안 쓰는 무거운 부분을 빼고 가볍게 포함하라는 설정
#  define WIN32_LEAN_AND_MEAN
// 윈도우 콘솔 설정 함수(SetConsoleOutputCP 등)를 쓰기 위한 헤더
#  include <windows.h>
// 윈도우 전용 블록을 닫는다
#endif

// 생성된 시간표를 저장할 기본 파일 이름
#define RESULT_FILE   "result_schedules.csv"

/*
 * Windows 콘솔에서 한글 UTF-8 출력이 깨지지 않도록 설정합니다.
 * #ifdef _WIN32 안의 코드는 Windows에서만 컴파일되고, macOS/Linux에서는 비어 있습니다.
 */
static void enable_utf8_console(void) {
// 아래 두 줄은 윈도우에서만 실행된다
#ifdef _WIN32
    // 콘솔 출력 글자코드를 UTF-8로 바꿔 한글이 안 깨지게 한다
    SetConsoleOutputCP(CP_UTF8);
    // 콘솔 입력 글자코드도 UTF-8로 맞춘다
    SetConsoleCP(CP_UTF8);
// 윈도우 전용 블록을 닫는다(맥/리눅스에서는 이 함수가 아무 일도 하지 않음)
#endif
}

/*
 * 테스트용 CSV 파일을 직접 만들어 줍니다.
 * FILE *fp는 파일을 가리키는 포인터입니다. fopen이 성공하면 파일에 쓸 수 있는
 * 통로를 돌려주고, 실패하면 NULL을 돌려줍니다.
 */
static void create_sample_csv(void) {
    // 샘플 파일을 쓰기 모드("w")로 연다
    FILE *fp = fopen("sample_courses.csv", "w");
    // 파일을 열지 못했으면
    if (!fp) {
        // 안내하고 함수를 끝낸다
        printf("  ※ 샘플 파일을 만들 수 없습니다.\n");
        return;
    }

    // Excel이 UTF-8 파일임을 알아보도록 BOM 3바이트를 먼저 쓴다
    fputc(0xEF, fp);
    fputc(0xBB, fp);
    fputc(0xBF, fp);

    // fprintf는 printf와 비슷하지만 화면이 아니라 fp가 가리키는 파일에 쓴다
    // 먼저 CSV 제목(헤더) 줄을 쓴다
    fprintf(fp, "강의명,교수명,요일,시작교시,종료교시,학점,평점,필수여부\n");
    // 아래는 예시 강의 10개를 한 줄씩 쓴 것이다
    fprintf(fp, "컴퓨터구조,김철수,월수,2,3,3,4.3,1\n");
    fprintf(fp, "운영체제,이영희,화목,3,4,3,4.1,1\n");
    fprintf(fp, "알고리즘,박민준,월수금,2,2,3,4.5,0\n");
    fprintf(fp, "데이터베이스,최지우,화목,5,6,3,3.9,0\n");
    fprintf(fp, "소프트웨어공학,정하늘,금,2,4,3,4.2,0\n");
    fprintf(fp, "네트워크,강도현,월수,5,6,3,3.7,0\n");
    fprintf(fp, "인공지능,윤서연,화목,1,2,3,4.8,0\n");
    fprintf(fp, "컴파일러,장우진,금,3,5,3,3.5,0\n");
    fprintf(fp, "임베디드시스템,한미래,월,6,8,2,4.0,0\n");
    fprintf(fp, "컴퓨터그래픽스,송지훈,수,3,5,2,4.4,0\n");
    // 파일을 닫는다
    fclose(fp);

    // 샘플 파일을 만들었음을 알려 준다
    printf("  ※ 샘플 파일 'sample_courses.csv' 생성 완료.\n\n");
}

/* 프로그램 제목과 간단한 안내 문구를 출력합니다. */
static void print_banner(void) {
    // 위쪽에 빈 줄 하나
    printf("\n");
    // 제목 상자의 윗변
    printf("╔══════════════════════════════════════════════════╗\n");
    // 제목 글자 줄
    printf("║       대학교 시간표 자동 생성기 v2.0             ║\n");
    // 제목 상자의 아랫변
    printf("╚══════════════════════════════════════════════════╝\n");
    // 프로그램이 무엇을 하는지 한 줄 안내
    printf("  · 강의를 불러온 뒤, 원하는 종류의 시간표를 자동으로 만들어 줍니다.\n");
    // 결과가 저장되는 파일 이름 안내
    printf("  · 만들어진 시간표는 '%s' 파일로도 저장됩니다.\n", RESULT_FILE);
}

/* 사용자가 선택할 수 있는 메뉴를 매 반복마다 화면에 보여 줍니다. */
static void print_menu(void) {
    // 메뉴 제목 줄
    printf("\n  ─── 메뉴 ─────────────────────────────────────\n");
    // [1] 강의를 직접 입력하는 항목
    printf("  [1] 강의 직접 입력\n");
    // [2] CSV 파일에서 강의를 불러오는 항목
    printf("  [2] CSV 파일에서 강의 불러오기\n");
    // [3] 샘플 CSV를 만들고 바로 불러오는 항목
    printf("  [3] 샘플 CSV 생성 후 불러오기\n");
    // [4] 현재 담긴 강의 목록을 보는 항목
    printf("  [4] 현재 강의 목록 보기\n");
    // 구분선
    printf("  ────────────────────────────────────────────────\n");
    // [5] 공강 하루 만들기(모드 1)
    printf("  [5] 모드 1 : 공강 하루 만들기\n");
    // [6] 1교시 없는 시간표(모드 2)
    printf("  [6] 모드 2 : 1교시 없는 시간표\n");
    // [7] 평점 높은 강의 위주(모드 3)
    printf("  [7] 모드 3 : 평점 높은 강의 위주\n");
    // [8] 세 모드를 한 번에 생성
    printf("  [8] 세 가지 모드 한 번에 생성\n");
    // 구분선
    printf("  ────────────────────────────────────────────────\n");
    // [0] 프로그램 종료
    printf("  [0] 종료\n");
    // 구분선
    printf("  ────────────────────────────────────────────────\n");
}

/* mode 번호를 사람이 읽기 쉬운 제목 문자열로 바꿉니다. */
static const char *mode_title(int mode) {
    // 모드 번호에 따라 알맞은 제목을 골라 돌려준다
    switch (mode) {
        // 모드 1의 제목
        case 1: return "[모드 1] 공강 하루 만들기 시간표";
        // 모드 2의 제목
        case 2: return "[모드 2] 1교시 없는 시간표";
        // 모드 3의 제목
        case 3: return "[모드 3] 평점 높은 강의 위주 시간표";
        // 그 밖의 값이면 기본 제목
        default: return "시간표";
    }
}

/*
 * 사용자가 고른 모드 번호에 맞는 최적화 함수를 호출합니다.
 * out은 결과를 받을 Schedule 구조체의 주소입니다. 함수 안에서 *out의 내용을 채우기
 * 때문에, 구조체를 복사해서 받지 않고 주소를 넘깁니다.
 */
static int generate(int mode, int max_credit, Schedule *out) {
    // 모드 번호에 따라 알맞은 최적화 함수를 부른다
    switch (mode) {
        // 모드 1: 공강 만들기
        case 1: return optimize_free_day(max_credit, out);
        // 모드 2: 1교시 없애기
        case 2: return optimize_no_first_period(max_credit, out);
        // 모드 3: 평점 우선
        case 3: return optimize_high_rating(max_credit, out);
        // 알 수 없는 모드면 실패(0)
        default: return 0;
    }
}

/* 모드 하나만 실행하고, 화면 출력과 CSV 저장까지 처리합니다. */
static void run_single_mode(int mode, int max_credit) {
    // 탐색을 시작한다고 알려 준다
    printf("\n  [진행] 최적 시간표를 탐색 중입니다...\n");

    // 결과 시간표를 받을 변수
    Schedule result;
    // 최적화를 시도했는데 조건에 맞는 시간표를 못 찾으면(0)
    if (!generate(mode, max_credit, &result)) {
        // 오류 안내를 보여 주고
        printf("\n  [오류] 조건을 만족하는 시간표를 찾지 못했습니다.\n");
        printf("     학점 한도를 늘리거나 필수 과목을 확인해 주세요.\n");
        // 함수를 끝낸다
        return;
    }

    // 이 모드에 맞는 제목 문자열을 가져온다
    const char *title = mode_title(mode);
    // 찾은 시간표를 화면에 보기 좋게 출력한다
    display_summary(&result, title);

    // save_schedules_csv는 여러 시간표를 저장할 수 있으므로, 1개짜리 배열로 넘긴다
    // 제목 1개를 담은 배열
    const char *titles[1] = { title };
    // 시간표 1개를 담은 배열
    Schedule   scheds[1] = { result };
    // CSV 저장에 성공하면
    if (save_schedules_csv(RESULT_FILE, titles, scheds, 1)) {
        // 저장 위치를 안내한다
        printf("\n  [저장] 결과를 '%s' 파일로 저장했습니다. (Excel로 열어 보세요)\n", RESULT_FILE);
    }
}

/* 세 가지 모드를 모두 실행하고, 성공한 결과들을 한 CSV 파일에 모아 저장합니다. */
static void run_all_modes(int max_credit) {
    // 성공한 시간표들을 담을 배열(최대 3개)
    Schedule    scheds[3];
    // 각 시간표의 제목을 담을 배열
    const char *titles[3];
    // 실제로 성공한 시간표 개수
    int n = 0;

    // 세 가지 시간표를 탐색한다고 알려 준다
    printf("\n  [진행] 세 가지 시간표를 탐색 중입니다...\n");

    // 모드 1, 2, 3을 차례로 실행한다
    for (int mode = 1; mode <= 3; mode++) {
        // 이번 모드의 결과를 받을 변수
        Schedule result;
        // 이번 모드가 성공하면(1)
        if (generate(mode, max_credit, &result)) {
            // 결과를 화면에 보여 주고
            display_summary(&result, mode_title(mode));
            // 저장 배열의 n번째 칸에 시간표를 담고
            scheds[n] = result;
            // 같은 칸에 제목을 담고
            titles[n] = mode_title(mode);
            // 성공 개수를 1 늘린다
            n++;
        } else {
            // 실패하면 어떤 모드가 실패했는지 안내한다
            printf("\n  [오류] %s: 조건을 만족하는 시간표를 찾지 못했습니다.\n",
                   mode_title(mode));
        }
        // 모드 사이를 굵은 구분선으로 나눈다
        printf("\n  ══════════════════════════════════\n");
    }

    // 성공한 시간표가 하나라도 있고, CSV 저장에도 성공하면
    if (n > 0 && save_schedules_csv(RESULT_FILE, titles, scheds, n)) {
        // 몇 개를 저장했는지 안내한다
        printf("\n  [저장] 시간표 %d개를 '%s' 파일로 저장했습니다. (Excel로 열어 보세요)\n",
               n, RESULT_FILE);
    }
}

/*
 * C 프로그램은 main 함수에서 시작합니다.
 * do-while 반복문은 메뉴를 최소 한 번 보여 준 뒤, 사용자가 0을 고를 때까지 반복합니다.
 */
int main(void) {
    // (윈도우라면) 콘솔을 UTF-8로 설정해 한글이 깨지지 않게 한다
    enable_utf8_console();
    // 프로그램 제목과 안내 문구를 보여 준다
    print_banner();

    // 사용자가 고른 메뉴 번호를 담을 변수
    int choice;
    // do-while: 안쪽을 먼저 한 번 실행한 뒤 조건을 확인해 반복 여부를 정한다
    do {
        // 메뉴를 보여 주고
        print_menu();
        // 0~8 사이의 번호를 입력받는다
        choice = read_int("  선택 > ", 0, 8);

        // switch는 choice 값에 따라 실행할 case를 고르는 문법이다
        switch (choice) {
            // [1] 강의를 직접 입력
            case 1:
                input_courses_manual();
                break;

            // [2] 사용자가 알려 준 CSV 파일에서 강의를 불러온다
            case 2: {
                // 파일명을 입력받을 문자열
                char fname[128];
                // 파일명을 입력받고
                read_line("  CSV 파일명 입력: ", fname, sizeof(fname));
                // 그 파일에서 강의를 불러온다
                load_courses_from_csv(fname);
                break;
            }

            // [3] 샘플 CSV를 만들고 바로 불러온다
            case 3:
                create_sample_csv();
                load_courses_from_csv("sample_courses.csv");
                break;

            // [4] 현재 담긴 강의 목록을 본다
            case 4:
                // 담긴 강의가 없으면 경고
                if (course_count == 0) {
                    printf("\n  [경고] 입력된 강의가 없습니다.\n");
                } else {
                    // 있으면 목록을 표로 출력
                    print_course_list();
                }
                break;

            // [5][6][7]은 처리 방식이 같아 함께 묶는다(아래 공통 코드로 떨어진다)
            case 5:
            case 6:
            case 7:
                // 강의가 없으면 먼저 불러오라고 안내
                if (course_count == 0) {
                    printf("\n  [경고] 먼저 강의를 입력하거나 불러오세요.\n");
                } else {
                    // 최대 학점을 입력받고
                    int mc = read_int("\n  최대 수강 학점을 입력하세요 (예: 18): ", 1, 99);
                    // 메뉴 번호에서 4를 빼면 모드 번호가 된다(5→1, 6→2, 7→3)
                    run_single_mode(choice - 4, mc);
                }
                break;

            // [8] 세 가지 모드를 한 번에 생성
            case 8:
                // 강의가 없으면 먼저 불러오라고 안내
                if (course_count == 0) {
                    printf("\n  [경고] 먼저 강의를 입력하거나 불러오세요.\n");
                } else {
                    // 최대 학점을 입력받고
                    int mc = read_int("\n  최대 수강 학점을 입력하세요 (예: 18): ", 1, 99);
                    // 세 모드를 모두 실행한다
                    run_all_modes(mc);
                }
                break;

            // [0] 종료를 골랐을 때
            case 0:
                printf("\n  프로그램을 종료합니다.\n\n");
                break;
        }
    // choice가 0이 아니면 메뉴로 돌아가 다시 반복한다
    } while (choice != 0);

    // 0을 돌려주며 프로그램을 정상 종료한다
    return 0;
}
