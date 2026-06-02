#include <stdio.h>
#include <string.h>
#include "display.h"

// 요약 출력에서 함께 쓰는 요일 이름입니다.
static const char *DAY_NAMES[] = {"월", "화", "수", "목", "금"};

// MARK: - 시간표 요약 출력

/**
 시간표의 요약 정보를 테두리 없이 간단한 목록 형태로 화면에 출력합니다.

 자세한 요일 × 교시 표는 결과 CSV 파일로 저장되고, 화면에는 핵심 요약만
 보여 줍니다.

 - Parameter s: 요약할 시간표
 - Parameter title: 요약 위에 표시할 제목
 */
void display_summary(const Schedule *s, const char *title) {
    printf("\n── %s ──\n", title);
    printf("  총 학점: %d학점   평균 평점: %.2f점\n",
           s->total_credit, s->avg_rating);

    // 어느 요일에 수업이 있는지 표시한 뒤, 비어 있는 요일을 공강으로 보여 줍니다.
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

        // 요일 인덱스들을 "월수" 같은 글자로 이어 붙입니다.
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
