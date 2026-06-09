#include <stdio.h>
#include <string.h>
#include "optimizer.h"

/*
 * optimizer.c
 * -----------
 * 가능한 강의 조합을 하나씩 만들어 보면서 조건에 가장 잘 맞는 시간표를 찾습니다.
 * 핵심 방식은 backtrack 함수의 재귀 호출입니다. 재귀는 함수가 자기 자신을 다시
 * 호출해서 "현재 선택에서 이어지는 다음 선택"을 탐색하는 방식입니다.
 */

#define SEARCH_WARN_THRESHOLD   25  /* 선택 과목이 이 개수보다 많으면 탐색 경고를 출력합니다. */

/* 시간표에 들어 있는 강의들의 평점 합계를 계산합니다. */
static double schedule_rating_sum(const Schedule *s) {
    double sum = 0.0;
    for (int i = 0; i < s->count; i++) {
        sum += course_list[s->indices[i]].rating;
    }
    return sum;
}

/* 평균은 평점 합계를 강의 수로 나누어 구합니다. 강의가 없으면 0으로 처리합니다. */
static double schedule_avg_rating(const Schedule *s) {
    return (s->count > 0) ? schedule_rating_sum(s) / s->count : 0.0;
}

/* 모든 필수 과목이 시간표 s 안에 포함되어 있는지 확인합니다. */
static int has_all_required(const Schedule *s) {
    for (int i = 0; i < course_count; i++) {
        if (!course_list[i].is_required) continue;

        int found = 0;
        for (int j = 0; j < s->count; j++) {
            if (s->indices[j] == i) {
                found = 1;
                break;
            }
        }
        if (!found) return 0;
    }
    return 1;
}

/* 모드별 추가 조건을 검사합니다. */
static int meets_mode_condition(const Schedule *s, int mode) {
    if (mode == 1) {
        return count_free_days(s) >= 1;
    }
    if (mode == 2) {
        return !has_period_1(s);
    }
    return 1;
}

/*
 * candidate가 현재 best보다 더 좋은 시간표인지 비교합니다.
 * 모드 1에서는 공강 요일 수를 먼저 보고, 이후에는 학점과 평균 평점을 봅니다.
 */
static int is_better_schedule(const Schedule *candidate,
                              const Schedule *best,
                              int mode,
                              int found_best) {
    if (!found_best) return 1;

    if (mode == 1) {
        int candidate_free_days = count_free_days(candidate);
        int best_free_days = count_free_days(best);

        if (candidate_free_days > best_free_days) return 1;
        if (candidate_free_days < best_free_days) return 0;
    }

    if (candidate->total_credit > best->total_credit) return 1;
    if (candidate->total_credit < best->total_credit) return 0;

    return schedule_avg_rating(candidate) > schedule_avg_rating(best);
}

/*
 * 백트래킹 탐색 함수입니다.
 * cur: 지금까지 선택한 시간표
 * start_idx: 다음에 살펴볼 course_list 시작 위치
 * best: 지금까지 찾은 가장 좋은 시간표를 저장할 위치
 * found_best: 좋은 시간표를 하나라도 찾았는지 알려 주는 int 변수의 주소
 */
static void backtrack(Schedule *cur,
                      int start_idx,
                      int mode,
                      int max_credit,
                      Schedule *best,
                      int *found_best) {
    /* 현재 선택이 조건을 만족한다면 best와 비교해 갱신합니다. */
    if (has_all_required(cur) && meets_mode_condition(cur, mode)) {
        if (is_better_schedule(cur, best, mode, *found_best)) {
            *found_best = 1;
            *best = *cur;
        }
    }

    if (cur->count >= MAX_SELECTED) {
        return;
    }

    for (int i = start_idx; i < course_count; i++) {
        Course *c = &course_list[i];

        if (cur->total_credit + c->credit > max_credit) continue;
        if (!can_add(cur, i)) continue;

        /* 강의를 하나 추가한 상태로 더 깊이 탐색합니다. */
        cur->indices[cur->count] = i;
        cur->count++;
        cur->total_credit += c->credit;

        backtrack(cur, i + 1, mode, max_credit, best, found_best);

        /*
         * 재귀 호출이 끝나면 방금 넣은 강의를 다시 빼서 원래 상태로 되돌립니다.
         * 그래야 다음 후보 강의를 넣어 보는 탐색을 계속할 수 있습니다.
         */
        cur->count--;
        cur->total_credit -= c->credit;
    }
}

/* 세 최적화 모드가 공통으로 사용하는 실제 탐색 준비 함수입니다. */
static int run_optimizer(int max_credit, Schedule *result, int mode) {
    Schedule best;
    Schedule cur;
    int found_best = 0;

    /* memset은 구조체 안의 바이트를 모두 0으로 채워 초기화합니다. */
    memset(&best, 0, sizeof(Schedule));
    memset(&cur, 0, sizeof(Schedule));

    /* 필수 과목은 반드시 들어가야 하므로 탐색 시작 전에 cur에 먼저 담습니다. */
    int required_count = 0;
    for (int i = 0; i < course_count; i++) {
        if (course_list[i].is_required) {
            if (cur.count >= MAX_SELECTED) {
                printf("\n[경고] 필수 과목이 너무 많습니다.\n");
                printf("   한 시간표에는 최대 %d개 강의만 담을 수 있습니다.\n",
                       MAX_SELECTED);
                return 0;
            }
            cur.indices[cur.count] = i;
            cur.count++;
            cur.total_credit += course_list[i].credit;
            required_count++;
        }
    }

    /* 필수 과목끼리 시간이 겹치면 어떤 선택 과목을 골라도 해결할 수 없습니다. */
    int ci, cj;
    if (!check_conflict(&cur, &ci, &cj)) {
        printf("\n[경고] 필수 과목끼리 시간이 겹칩니다!\n");
        printf("   - %s  vs  %s\n", course_list[ci].name, course_list[cj].name);
        printf("   먼저 필수 과목 충돌을 해결해 주세요.\n");
        return 0;
    }

    if (cur.total_credit > max_credit) {
        printf("\n[경고] 필수 과목만으로도 %d학점이라 한도(%d학점)를 넘습니다.\n",
               cur.total_credit, max_credit);
        return 0;
    }

    int elective_count = course_count - required_count;
    if (elective_count > SEARCH_WARN_THRESHOLD) {
        printf("\n  ※ 선택 과목이 %d개로 많아 탐색이 다소 오래 걸릴 수 있습니다.\n",
               elective_count);
    }

    backtrack(&cur, 0, mode, max_credit, &best, &found_best);

    if (!found_best) return 0;

    *result = best;
    compute_stats(result);
    return 1;
}

/* 아래 세 함수는 main.c에서 모드별로 호출하기 좋은 이름을 제공하는 래퍼입니다. */
int optimize_free_day(int max_credit, Schedule *result) {
    return run_optimizer(max_credit, result, 1);
}

int optimize_no_first_period(int max_credit, Schedule *result) {
    return run_optimizer(max_credit, result, 2);
}

int optimize_high_rating(int max_credit, Schedule *result) {
    return run_optimizer(max_credit, result, 3);
}
