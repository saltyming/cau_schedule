#include <stdio.h>
#include <string.h>
#include "schedule.h"

/*
 * schedule.c
 * ----------
 * 이미 선택된 강의들이 서로 겹치는지, 특정 요일/교시에 어떤 강의가 있는지,
 * 총 학점과 평균 평점을 계산하는 함수들을 구현합니다.
 */

/*
 * 두 강의 번호 a, b가 시간상 겹치는지 검사합니다.
 * a와 b는 Course 구조체 자체가 아니라 course_list 배열의 인덱스입니다.
 */
static int courses_overlap(int a, int b) {
    Course *ca = &course_list[a];
    Course *cb = &course_list[b];

    /*
     * &course_list[a]는 "course_list 배열 a번째 칸의 주소"입니다.
     * ca->day_count는 (*ca).day_count와 같은 뜻으로, 포인터가 가리키는 구조체의
     * 멤버에 접근할 때 쓰는 C 문법입니다.
     */
    for (int da = 0; da < ca->day_count; da++) {
        for (int db = 0; db < cb->day_count; db++) {
            if (ca->days[da] != cb->days[db]) continue;

            /*
             * 같은 요일일 때만 교시가 겹치는지 봅니다.
             * 두 구간 [시작, 종료]가 겹치려면 A의 시작이 B의 종료 이하이고,
             * B의 시작이 A의 종료 이하이면 됩니다.
             */
            if (ca->start_period <= cb->end_period &&
                cb->start_period <= ca->end_period) {
                return 1;
            }
        }
    }
    return 0;
}

/*
 * 시간표 s 안의 모든 강의 쌍을 비교해 충돌이 있는지 확인합니다.
 * ci, cj는 충돌한 강의 번호를 호출한 쪽에 알려 주기 위한 출력용 포인터입니다.
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

/* 새 강의 course_idx를 현재 시간표 s에 넣어도 시간 충돌이 없는지 확인합니다. */
int can_add(const Schedule *s, int course_idx) {
    for (int i = 0; i < s->count; i++) {
        if (courses_overlap(s->indices[i], course_idx)) return 0;
    }
    return 1;
}

/* 시간표에 1교시에 시작하는 강의가 하나라도 있으면 1을 반환합니다. */
int has_period_1(const Schedule *s) {
    for (int i = 0; i < s->count; i++) {
        if (course_list[s->indices[i]].start_period == 1) return 1;
    }
    return 0;
}

/* 월~금 중 수업이 하나도 없는 요일 수를 셉니다. */
int count_free_days(const Schedule *s) {
    int used[DAYS_IN_WEEK] = {0};

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

/*
 * 특정 요일(day)과 교시(period)에 들어 있는 강의를 찾습니다.
 * 찾으면 course_list의 인덱스를 반환하고, 비어 있으면 -1을 반환합니다.
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

/* 선택된 강의들의 총 학점과 평균 평점을 Schedule 안에 계산해 넣습니다. */
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
