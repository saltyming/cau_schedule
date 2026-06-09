// 화면 출력(printf 등)을 위한 표준 입출력 라이브러리
#include <stdio.h>
// 문자열·메모리 다루기를 위한 라이브러리
#include <string.h>
// 이 파일에서 쓰는 Schedule, Course 구조체와 함수 약속이 담긴 헤더
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
    // a번 강의 정보가 있는 칸의 주소를 가져온다(&course_list[a] = a번째 칸의 주소)
    Course *ca = &course_list[a];
    // b번 강의 정보가 있는 칸의 주소를 가져온다
    Course *cb = &course_list[b];

    // ca->day_count 는 (*ca).day_count 와 같은 뜻으로, 포인터가 가리키는 구조체의 멤버를 읽는 문법이다
    // a 강의가 수업하는 요일을 하나씩 살펴본다
    for (int da = 0; da < ca->day_count; da++) {
        // 그 요일마다 b 강의의 모든 요일과 짝지어 비교한다
        for (int db = 0; db < cb->day_count; db++) {
            // 두 강의의 요일이 서로 다르면 겹칠 수 없으니 다음 비교로 넘어간다
            if (ca->days[da] != cb->days[db]) continue;

            // 같은 요일일 때만 교시(시간)가 겹치는지 본다.
            // 두 구간 [시작, 종료]가 겹치려면 a의 시작이 b의 종료 이하이고,
            if (ca->start_period <= cb->end_period &&
                // b의 시작도 a의 종료 이하이면 된다
                cb->start_period <= ca->end_period) {
                // 두 조건을 만족하면 시간이 겹치므로 1(겹침)을 돌려준다
                return 1;
            }
        }
    }
    // 모든 요일을 비교했지만 겹치는 시간이 없으면 0(안 겹침)
    return 0;
}

/*
 * 시간표 s 안의 모든 강의 쌍을 비교해 충돌이 있는지 확인합니다.
 * ci, cj는 충돌한 강의 번호를 호출한 쪽에 알려 주기 위한 출력용 포인터입니다.
 */
int check_conflict(const Schedule *s, int *ci, int *cj) {
    // 시간표의 강의를 앞에서부터 하나씩 고른다
    for (int i = 0; i < s->count; i++) {
        // 그 강의보다 뒤에 있는 강의들과만 짝지어 본다(모든 쌍을 한 번씩 비교)
        for (int j = i + 1; j < s->count; j++) {
            // 두 강의(i번째, j번째 선택)가 시간상 겹치면
            if (courses_overlap(s->indices[i], s->indices[j])) {
                // ci 가 주어졌으면 첫 번째 충돌 강의 번호를 적어 준다
                if (ci) *ci = s->indices[i];
                // cj 가 주어졌으면 두 번째 충돌 강의 번호를 적어 준다
                if (cj) *cj = s->indices[j];
                // 충돌이 있으니 0(문제 있음)을 돌려준다
                return 0;
            }
        }
    }
    // 어떤 쌍도 겹치지 않으면 1(문제 없음)을 돌려준다
    return 1;
}

/* 새 강의 course_idx를 현재 시간표 s에 넣어도 시간 충돌이 없는지 확인합니다. */
int can_add(const Schedule *s, int course_idx) {
    // 시간표에 이미 들어 있는 강의들을 하나씩 본다
    for (int i = 0; i < s->count; i++) {
        // 새 강의와 겹치는 게 하나라도 있으면 넣을 수 없으므로 0
        if (courses_overlap(s->indices[i], course_idx)) return 0;
    }
    // 아무와도 안 겹치면 넣어도 된다는 뜻으로 1
    return 1;
}

/* 시간표에 1교시에 시작하는 강의가 하나라도 있으면 1을 반환합니다. */
int has_period_1(const Schedule *s) {
    // 선택된 강의들을 하나씩 살펴본다
    for (int i = 0; i < s->count; i++) {
        // 그 강의의 시작 교시가 1이면 1교시 수업이 있는 것이므로 1
        if (course_list[s->indices[i]].start_period == 1) return 1;
    }
    // 1교시 강의가 하나도 없으면 0
    return 0;
}

/* 월~금 중 수업이 하나도 없는 요일 수를 셉니다. */
int count_free_days(const Schedule *s) {
    // 월~금 각 요일을 썼는지 표시할 배열(처음엔 모두 0 = 안 씀)
    int used[DAYS_IN_WEEK] = {0};

    // 선택된 강의들을 하나씩 본다
    for (int i = 0; i < s->count; i++) {
        // i번째로 선택한 강의의 주소를 가져온다
        Course *c = &course_list[s->indices[i]];
        // 그 강의가 수업하는 요일들을 하나씩 본다
        for (int d = 0; d < c->day_count; d++) {
            // 올바른 요일 번호면 그 요일을 '썼음'(1)으로 표시한다
            if (c->days[d] >= 0) used[c->days[d]] = 1;
        }
    }

    // 공강(빈 요일) 개수를 셀 변수
    int free_days = 0;
    // 월~금을 하나씩 보며
    for (int d = 0; d < DAYS_IN_WEEK; d++) {
        // 그 요일을 한 번도 안 썼으면 공강이므로 1 더한다
        if (!used[d]) free_days++;
    }
    // 공강 요일 수를 돌려준다
    return free_days;
}

/*
 * 특정 요일(day)과 교시(period)에 들어 있는 강의를 찾습니다.
 * 찾으면 course_list의 인덱스를 반환하고, 비어 있으면 -1을 반환합니다.
 */
int course_at(const Schedule *s, int day, int period) {
    // 선택된 강의들을 하나씩 살펴본다
    for (int i = 0; i < s->count; i++) {
        // i번째로 선택한 강의의 주소를 가져온다
        Course *c = &course_list[s->indices[i]];
        // 그 강의가 수업하는 요일들을 하나씩 본다
        for (int d = 0; d < c->day_count; d++) {
            // 찾는 요일과 같고,
            if (c->days[d] == day &&
                // 찾는 교시가 이 강의의 시작 교시 이상이고,
                period >= c->start_period &&
                // 종료 교시 이하이면(즉 그 칸이 이 강의 수업 시간이면)
                period <= c->end_period) {
                // 그 강의의 번호를 돌려준다
                return s->indices[i];
            }
        }
    }
    // 그 요일·교시 칸이 비어 있으면 -1(없음)
    return -1;
}

/* 선택된 강의들의 총 학점과 평균 평점을 Schedule 안에 계산해 넣습니다. */
void compute_stats(Schedule *s) {
    // 총 학점을 0에서부터 더해 나간다
    s->total_credit = 0;
    // 평점들의 합을 담을 변수(0.0에서 시작)
    float rating_sum = 0.0f;

    // 선택된 강의들을 하나씩 본다
    for (int i = 0; i < s->count; i++) {
        // i번째로 선택한 강의의 주소를 가져온다
        Course *c = &course_list[s->indices[i]];
        // 그 강의의 학점을 총 학점에 더한다
        s->total_credit += c->credit;
        // 그 강의의 평점을 평점 합에 더한다
        rating_sum      += c->rating;
    }
    // 평균 평점 = 평점 합 ÷ 강의 수. 단, 강의가 0개면 0으로 둔다(0으로 나누기 방지)
    s->avg_rating = (s->count > 0) ? rating_sum / s->count : 0.0f;
}
