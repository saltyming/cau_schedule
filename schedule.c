#include <stdio.h>
#include <string.h>
#include "schedule.h"

// MARK: - 시간 겹침 검사

/**
 두 강의가 같은 시간에 겹치는지 확인합니다.

 같은 요일에 교시 범위가 한 칸이라도 닿으면
 함께 들을 수 없는 강의로 판단합니다.

 - Parameter a: 첫 번째 강의의 course_list 인덱스
 - Parameter b: 두 번째 강의의 course_list 인덱스
 - Returns: 겹치면 1, 겹치지 않으면 0.
 */
static int courses_overlap(int a, int b) {
    Course *ca = &course_list[a];
    Course *cb = &course_list[b];

    for (int da = 0; da < ca->day_count; da++) {
        for (int db = 0; db < cb->day_count; db++) {
            if (ca->days[da] != cb->days[db]) continue;

            // 같은 요일이라면 교시 구간이 겹치는지 확인합니다.
            // (시작 ≤ 상대의 끝) 이고 (상대의 시작 ≤ 끝) 이면 겹칩니다.
            if (ca->start_period <= cb->end_period &&
                cb->start_period <= ca->end_period) {
                return 1;
            }
        }
    }
    return 0;
}

/**
 시간표 안에서 시간이 겹치는 첫 강의 쌍을 찾습니다.

 - Parameter s: 검사할 시간표
 - Parameter ci: (결과 전달용) 겹치는 첫 번째 강의 인덱스
 - Parameter cj: (결과 전달용) 겹치는 두 번째 강의 인덱스
 - Returns: 겹침이 없으면 1, 있으면 0 (ci/cj에 겹친 강의를 담아 줌).
 */
int check_conflict(const Schedule *s, int *ci, int *cj) {
    for (int i = 0; i < s->count; i++) {
        for (int j = i + 1; j < s->count; j++) {
            if (courses_overlap(s->indices[i], s->indices[j])) {
                if (ci) *ci = s->indices[i];
                if (cj) *cj = s->indices[j];
                return 0;
            }
        }
    }
    return 1;
}

/**
 어떤 강의를 시간표에 더해도 다른 강의와 겹치지 않는지 확인합니다.

 - Parameter s: 대상 시간표
 - Parameter course_idx: 추가하려는 강의의 course_list 인덱스
 - Returns: 겹치지 않아 추가할 수 있으면 1, 아니면 0.
 */
int can_add(const Schedule *s, int course_idx) {
    for (int i = 0; i < s->count; i++) {
        if (courses_overlap(s->indices[i], course_idx)) return 0;
    }
    return 1;
}

// MARK: - 시간표 속성 계산

/**
 시간표에 1교시 수업이 하나라도 있는지 확인합니다.

 - Parameter s: 검사할 시간표
 - Returns: 1교시 수업이 있으면 1, 없으면 0.
 */
int has_period_1(const Schedule *s) {
    for (int i = 0; i < s->count; i++) {
        if (course_list[s->indices[i]].start_period == 1) return 1;
    }
    return 0;
}

/**
 시간표에서 수업이 하나도 없는 '공강 요일'의 수를 셉니다.

 - Parameter s: 검사할 시간표
 - Returns: 월~금 중 수업이 전혀 없는 요일의 수.
 */
int count_free_days(const Schedule *s) {
    int used[DAYS_IN_WEEK] = {0};   // 요일별로 수업이 있으면 1로 표시

    for (int i = 0; i < s->count; i++) {
        Course *c = &course_list[s->indices[i]];
        for (int d = 0; d < c->day_count; d++) {
            if (c->days[d] >= 0) used[c->days[d]] = 1;
        }
    }

    int free_days = 0;
    for (int d = 0; d < DAYS_IN_WEEK; d++) {
        if (!used[d]) free_days++;
    }
    return free_days;
}

/**
 특정 요일·교시 칸을 차지하고 있는 강의를 찾습니다.

 - Parameter s: 대상 시간표
 - Parameter day: 요일 인덱스 (0=월 … 4=금)
 - Parameter period: 교시 번호 (1~9)
 - Returns: 그 칸을 차지한 강의의 course_list 인덱스. 비어 있으면 -1.
 */
int course_at(const Schedule *s, int day, int period) {
    for (int i = 0; i < s->count; i++) {
        Course *c = &course_list[s->indices[i]];
        for (int d = 0; d < c->day_count; d++) {
            if (c->days[d] == day &&
                period >= c->start_period &&
                period <= c->end_period) {
                return s->indices[i];
            }
        }
    }
    return -1;
}

/**
 시간표의 요약값(총 학점·평균 평점)을 계산해 채워 넣습니다.

 - Parameter s: 요약값을 갱신할 시간표
 */
void compute_stats(Schedule *s) {
    s->total_credit = 0;
    float rating_sum = 0.0f;

    for (int i = 0; i < s->count; i++) {
        Course *c = &course_list[s->indices[i]];
        s->total_credit += c->credit;
        rating_sum      += c->rating;
    }
    s->avg_rating = (s->count > 0) ? rating_sum / s->count : 0.0f;
}
