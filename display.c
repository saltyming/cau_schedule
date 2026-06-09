// 화면 출력(printf 등)을 위한 표준 입출력 라이브러리
#include <stdio.h>
// 문자열 이어 붙이기(strcat) 등을 위한 라이브러리
#include <string.h>
// 이 파일에서 쓰는 display_summary 약속과 Schedule 구조체가 담긴 헤더
#include "display.h"

/*
 * display.c
 * ---------
 * 최적화가 끝난 Schedule을 사람이 읽기 좋은 콘솔 출력으로 바꿉니다.
 */

// 요일 번호 0~4를 화면에 찍을 한글 문자열로 바꾸기 위한 배열
static const char *DAY_NAMES[] = {"월", "화", "수", "목", "금"};

/* 완성된 시간표의 요약, 공강 요일, 1교시 여부, 강의 목록을 출력합니다. */
void display_summary(const Schedule *s, const char *title) {
    // 제목 줄을 출력한다(예: "── 공강 하루 만들기 ──")
    printf("\n── %s ──\n", title);
    // 총 학점과 평균 평점을 출력한다(평점은 소수 둘째 자리까지)
    printf("  총 학점: %d학점   평균 평점: %.2f점\n",
           s->total_credit, s->avg_rating);

    // used[d]가 1이면 d번째 요일에 수업이 있다는 뜻(처음엔 모두 0)
    int used[DAYS_IN_WEEK] = {0};
    // 선택된 강의들을 하나씩 보며
    for (int i = 0; i < s->count; i++) {
        // i번째로 선택한 강의의 주소를 가져온다
        Course *c = &course_list[s->indices[i]];
        // 그 강의가 수업하는 요일들을 모두 '수업 있음'(1)으로 표시한다
        for (int d = 0; d < c->day_count; d++) used[c->days[d]] = 1;
    }

    // 공강(빈 요일) 개수를 구한다
    int free_days = count_free_days(s);
    // 공강 일수와 함께 어떤 요일이 비는지 괄호로 나열을 시작한다
    printf("  공강 요일: %d일  (", free_days);
    // 월~금을 하나씩 보며
    for (int d = 0; d < DAYS_IN_WEEK; d++) {
        // 수업이 없는 요일이면 그 이름을 출력한다
        if (!used[d]) printf("%s ", DAY_NAMES[d]);
    }
    // 나열을 닫는 괄호로 마무리한다
    printf(")\n");

    // 1교시 강의가 있는지 "있음/없음"으로 알려 준다
    printf("  1교시 강의: %s\n", has_period_1(s) ? "있음" : "없음");

    // 선택된 강의들의 자세한 목록을 출력한다
    printf("  수강 강의 목록:\n");
    // 선택된 강의들을 하나씩 보며
    for (int i = 0; i < s->count; i++) {
        // i번째로 선택한 강의의 주소를 가져온다
        Course *c = &course_list[s->indices[i]];

        // 한 강의가 월수처럼 여러 요일에 열리면 day_str에 요일 글자를 이어 붙인다(빈 문자열에서 시작)
        char day_str[12] = "";
        // 그 강의의 요일들을 하나씩 보며
        for (int d = 0; d < c->day_count; d++) {
            // 요일 글자를 day_str 뒤에 이어 붙인다
            strcat(day_str, DAY_NAMES[c->days[d]]);
        }
        // 강의명·요일·교시·평점·학점·필수 여부를 한 줄로 정리해 출력한다
        // (%-18s 는 강의명을 18칸 너비로 왼쪽 정렬한다는 뜻)
        printf("    %-18s %s %d~%d교시 %.1f점 %d학점 %s\n",
               c->name, day_str,
               c->start_period, c->end_period,
               c->rating, c->credit,
               c->is_required ? "[필수]" : "");
    }
}
