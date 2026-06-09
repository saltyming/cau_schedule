#include <stdio.h>
#include <string.h>
#include "display.h"

/*
 * display.c
 * ---------
 * 최적화가 끝난 Schedule을 사람이 읽기 좋은 콘솔 출력으로 바꿉니다.
 */

/* 요일 번호 0~4를 화면에 찍을 한글 문자열로 바꾸기 위한 배열입니다. */
static const char *DAY_NAMES[] = {"월", "화", "수", "목", "금"};

/* 완성된 시간표의 요약, 공강 요일, 1교시 여부, 강의 목록을 출력합니다. */
void display_summary(const Schedule *s, const char *title) {
    printf("\n── %s ──\n", title);
    printf("  총 학점: %d학점   평균 평점: %.2f점\n",
           s->total_credit, s->avg_rating);

    /* used[d]가 1이면 d번째 요일에 수업이 있다는 뜻입니다. */
    int used[DAYS_IN_WEEK] = {0};
    for (int i = 0; i < s->count; i++) {
        Course *c = &course_list[s->indices[i]];
        for (int d = 0; d < c->day_count; d++) used[c->days[d]] = 1;
    }

    int free_days = count_free_days(s);
    printf("  공강 요일: %d일  (", free_days);
    for (int d = 0; d < DAYS_IN_WEEK; d++) {
        if (!used[d]) printf("%s ", DAY_NAMES[d]);
    }
    printf(")\n");

    printf("  1교시 강의: %s\n", has_period_1(s) ? "있음" : "없음");

    printf("  수강 강의 목록:\n");
    for (int i = 0; i < s->count; i++) {
        Course *c = &course_list[s->indices[i]];

        /* 한 강의가 월수처럼 여러 요일에 열리면 day_str에 이어 붙입니다. */
        char day_str[12] = "";
        for (int d = 0; d < c->day_count; d++) {
            strcat(day_str, DAY_NAMES[c->days[d]]);
        }
        printf("    %-18s %s %d~%d교시 %.1f점 %d학점 %s\n",
               c->name, day_str,
               c->start_period, c->end_period,
               c->rating, c->credit,
               c->is_required ? "[필수]" : "");
    }
}
