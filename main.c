#include <stdio.h>
#include "course.h"
#include "schedule.h"
#include "optimizer.h"
#include "display.h"
#include "export.h"

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif

// 만들어진 시간표 결과를 저장하는 파일 이름입니다. Excel로 바로 열 수 있습니다.
#define RESULT_FILE   "result_schedules.csv"

// MARK: - 플랫폼별 초기화

/**
 윈도우 콘솔에서 한글이 깨지지 않도록 입출력 인코딩을 UTF-8로 맞춥니다.

 윈도우 명령 프롬프트는 기본 인코딩이 UTF-8이 아니어서, 이 설정이 없으면
 한글이 깨져 보일 수 있습니다. 맥·리눅스는 기본이 UTF-8이라 아무 일도 하지
 않습니다.
 */
static void enable_utf8_console(void) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);   // 화면에 출력하는 글자를 UTF-8로
    SetConsoleCP(CP_UTF8);         // 키보드로 입력받는 글자를 UTF-8로
#endif
}

// MARK: - 샘플 데이터 만들기

/**
 처음 사용해 보는 사람을 위해 예제 강의가 담긴 CSV 파일을 만듭니다.

 한국어 Excel에서도 한글이 깨지지 않도록 파일 맨 앞에 UTF-8 표식(BOM)을
 넣습니다.
 */
static void create_sample_csv(void) {
    FILE *fp = fopen("sample_courses.csv", "w");
    if (!fp) {
        printf("  ※ 샘플 파일을 만들 수 없습니다.\n");
        return;
    }

    // UTF-8 표식(BOM) — Excel이 한글을 올바르게 읽게 합니다.
    fputc(0xEF, fp);
    fputc(0xBB, fp);
    fputc(0xBF, fp);

    fprintf(fp, "강의명,교수명,요일,시작교시,종료교시,학점,평점,필수여부\n");
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
    fclose(fp);

    printf("  ※ 샘플 파일 'sample_courses.csv' 생성 완료.\n\n");
}

// MARK: - 화면 안내

/// 프로그램 시작 화면(제목)을 출력합니다.
static void print_banner(void) {
    printf("\n");
    printf("╔══════════════════════════════════════════════════╗\n");
    printf("║       🎓 대학교 시간표 자동 생성기 v2.0          ║\n");
    printf("╚══════════════════════════════════════════════════╝\n");
    printf("  · 강의를 불러온 뒤, 원하는 종류의 시간표를 자동으로 만들어 줍니다.\n");
    printf("  · 만들어진 시간표는 '%s' 파일로도 저장됩니다.\n", RESULT_FILE);
}

/// 메뉴 목록을 출력합니다. (선택 입력은 read_int가 따로 받습니다)
static void print_menu(void) {
    printf("\n  ─── 메뉴 ─────────────────────────────────────\n");
    printf("  [1] 강의 직접 입력\n");
    printf("  [2] CSV 파일에서 강의 불러오기\n");
    printf("  [3] 샘플 CSV 생성 후 불러오기\n");
    printf("  [4] 현재 강의 목록 보기\n");
    printf("  ────────────────────────────────────────────────\n");
    printf("  [5] 모드 1 : 공강 하루 만들기\n");
    printf("  [6] 모드 2 : 1교시 없는 시간표\n");
    printf("  [7] 모드 3 : 평점 높은 강의 위주\n");
    printf("  [8] 세 가지 모드 한 번에 생성\n");
    printf("  ────────────────────────────────────────────────\n");
    printf("  [0] 종료\n");
    printf("  ────────────────────────────────────────────────\n");
}

// MARK: - 시간표 생성 실행

/**
 모드 번호에 맞는 제목 문구를 돌려줍니다.

 - Parameter mode: 모드 번호 (1~3)
 - Returns: 화면·파일에 쓸 제목 문자열.
 */
static const char *mode_title(int mode) {
    switch (mode) {
        case 1: return "[모드 1] 공강 하루 만들기 시간표";
        case 2: return "[모드 2] 1교시 없는 시간표";
        case 3: return "[모드 3] 평점 높은 강의 위주 시간표";
        default: return "시간표";
    }
}

/**
 모드 번호에 맞는 시간표 생성 함수를 골라 실행합니다.

 - Parameter mode: 모드 번호 (1~3)
 - Parameter max_credit: 학점 한도
 - Parameter out: 만들어진 시간표를 담을 곳
 - Returns: 시간표를 찾으면 1, 못 찾으면 0.
 */
static int generate(int mode, int max_credit, Schedule *out) {
    switch (mode) {
        case 1: return optimize_free_day(max_credit, out);
        case 2: return optimize_no_first_period(max_credit, out);
        case 3: return optimize_high_rating(max_credit, out);
        default: return 0;
    }
}

/**
 한 가지 모드의 시간표를 만들어 화면에 보여 주고 파일로 저장합니다.

 - Parameter mode: 모드 번호 (1~3)
 - Parameter max_credit: 학점 한도
 */
static void run_single_mode(int mode, int max_credit) {
    printf("\n  🔍 최적 시간표를 탐색 중입니다...\n");

    Schedule result;
    if (!generate(mode, max_credit, &result)) {
        printf("\n  ❌ 조건을 만족하는 시간표를 찾지 못했습니다.\n");
        printf("     학점 한도를 늘리거나 필수 과목을 확인해 주세요.\n");
        return;
    }

    const char *title = mode_title(mode);
    display_summary(&result, title);

    // 화면에 보여 준 시간표를 파일로도 저장합니다.
    const char *titles[1] = { title };
    Schedule   scheds[1] = { result };
    if (save_schedules_csv(RESULT_FILE, titles, scheds, 1)) {
        printf("\n  💾 결과를 '%s' 파일로 저장했습니다. (Excel로 열어 보세요)\n", RESULT_FILE);
    }
}

/**
 세 가지 모드의 시간표를 모두 만들어 화면에 보여 주고,
 성공한 시간표들을 한 파일에 함께 저장합니다.

 - Parameter max_credit: 학점 한도
 */
static void run_all_modes(int max_credit) {
    Schedule    scheds[3];
    const char *titles[3];
    int n = 0;   // 실제로 만들어진 시간표 개수

    printf("\n  🔍 세 가지 시간표를 탐색 중입니다...\n");

    for (int mode = 1; mode <= 3; mode++) {
        Schedule result;
        if (generate(mode, max_credit, &result)) {
            display_summary(&result, mode_title(mode));
            scheds[n] = result;
            titles[n] = mode_title(mode);
            n++;
        } else {
            printf("\n  ❌ %s: 조건을 만족하는 시간표를 찾지 못했습니다.\n",
                   mode_title(mode));
        }
        printf("\n  ══════════════════════════════════\n");
    }

    if (n > 0 && save_schedules_csv(RESULT_FILE, titles, scheds, n)) {
        printf("\n  💾 시간표 %d개를 '%s' 파일로 저장했습니다. (Excel로 열어 보세요)\n",
               n, RESULT_FILE);
    }
}

// MARK: - 프로그램 시작점

/**
 메뉴를 반복해서 보여 주며 사용자의 선택을 처리합니다.

 - Returns: 프로그램 종료 코드(항상 0).
 */
int main(void) {
    enable_utf8_console();   // (윈도우) 콘솔 한글 깨짐 방지
    print_banner();

    int choice;
    do {
        print_menu();
        choice = read_int("  선택 > ", 0, 8);

        switch (choice) {
            case 1:
                input_courses_manual();
                break;

            case 2: {
                char fname[128];
                read_line("  CSV 파일명 입력: ", fname, sizeof(fname));
                load_courses_from_csv(fname);
                break;
            }

            case 3:
                create_sample_csv();
                load_courses_from_csv("sample_courses.csv");
                break;

            case 4:
                if (course_count == 0) {
                    printf("\n  ⚠ 입력된 강의가 없습니다.\n");
                } else {
                    print_course_list();
                }
                break;

            case 5:
            case 6:
            case 7:
                if (course_count == 0) {
                    printf("\n  ⚠ 먼저 강의를 입력하거나 불러오세요.\n");
                } else {
                    int mc = read_int("\n  최대 수강 학점을 입력하세요 (예: 18): ", 1, 99);
                    run_single_mode(choice - 4, mc);
                }
                break;

            case 8:
                if (course_count == 0) {
                    printf("\n  ⚠ 먼저 강의를 입력하거나 불러오세요.\n");
                } else {
                    int mc = read_int("\n  최대 수강 학점을 입력하세요 (예: 18): ", 1, 99);
                    run_all_modes(mc);
                }
                break;

            case 0:
                printf("\n  프로그램을 종료합니다. 👋\n\n");
                break;
        }
    } while (choice != 0);

    return 0;
}
