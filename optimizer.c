// 화면 출력(printf 등)을 위한 표준 입출력 라이브러리
#include <stdio.h>
// 메모리 초기화(memset) 등을 위한 라이브러리
#include <string.h>
// 이 파일에서 쓰는 최적화 함수 약속과 Schedule 구조체가 담긴 헤더
#include "optimizer.h"

/*
 * optimizer.c
 * -----------
 * 가능한 강의 조합을 하나씩 만들어 보면서 조건에 가장 잘 맞는 시간표를 찾습니다.
 * 핵심 방식은 backtrack 함수의 재귀 호출입니다. 재귀는 함수가 자기 자신을 다시
 * 호출해서 "현재 선택에서 이어지는 다음 선택"을 탐색하는 방식입니다.
 */

// 선택 과목이 이 개수보다 많으면 탐색이 오래 걸릴 수 있어 경고를 출력한다
#define SEARCH_WARN_THRESHOLD   25

/* 시간표에 들어 있는 강의들의 평점 합계를 계산합니다. */
static double schedule_rating_sum(const Schedule *s) {
    // 평점을 더해 나갈 변수(0.0에서 시작)
    double sum = 0.0;
    // 선택된 강의들을 하나씩 보며
    for (int i = 0; i < s->count; i++) {
        // 그 강의의 평점을 합에 더한다
        sum += course_list[s->indices[i]].rating;
    }
    // 평점 총합을 돌려준다
    return sum;
}

/* 평균은 평점 합계를 강의 수로 나누어 구합니다. 강의가 없으면 0으로 처리합니다. */
static double schedule_avg_rating(const Schedule *s) {
    // 강의가 1개 이상이면 합÷개수, 0개면 0으로 둔다(0으로 나누기 방지)
    return (s->count > 0) ? schedule_rating_sum(s) / s->count : 0.0;
}

/* 모든 필수 과목이 시간표 s 안에 포함되어 있는지 확인합니다. */
static int has_all_required(const Schedule *s) {
    // 전체 강의를 하나씩 살펴본다
    for (int i = 0; i < course_count; i++) {
        // 필수 과목이 아니면 검사할 필요가 없으니 건너뛴다
        if (!course_list[i].is_required) continue;

        // 이 필수 과목을 시간표에서 찾았는지 표시(처음엔 0=못 찾음)
        int found = 0;
        // 시간표에 선택된 강의들을 하나씩 보며
        for (int j = 0; j < s->count; j++) {
            // 그중 이 필수 과목 번호(i)가 있으면
            if (s->indices[j] == i) {
                // 찾았다고 표시하고
                found = 1;
                // 더 볼 필요 없으니 안쪽 반복을 멈춘다
                break;
            }
        }
        // 이 필수 과목이 시간표에 없으면 0(빠진 게 있음)을 돌려준다
        if (!found) return 0;
    }
    // 모든 필수 과목이 들어 있으면 1
    return 1;
}

/* 모드별 추가 조건을 검사합니다. */
static int meets_mode_condition(const Schedule *s, int mode) {
    // 모드 1(공강 만들기): 공강 요일이 하나 이상이어야 한다
    if (mode == 1) {
        return count_free_days(s) >= 1;
    }
    // 모드 2(1교시 없애기): 1교시 수업이 없어야 한다
    if (mode == 2) {
        return !has_period_1(s);
    }
    // 모드 3(평점 우선): 특별한 추가 조건이 없으므로 항상 통과(1)
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
    // 아직 좋은 시간표를 하나도 못 찾았다면, 지금 후보가 무조건 더 좋다
    if (!found_best) return 1;

    // 모드 1에서는 공강 요일 수를 가장 먼저 따진다
    if (mode == 1) {
        // 후보의 공강 요일 수
        int candidate_free_days = count_free_days(candidate);
        // 기존 best의 공강 요일 수
        int best_free_days = count_free_days(best);

        // 후보의 공강이 더 많으면 더 좋다(1)
        if (candidate_free_days > best_free_days) return 1;
        // 후보의 공강이 더 적으면 더 나쁘다(0)
        if (candidate_free_days < best_free_days) return 0;
    }

    // 공강이 같거나 다른 모드라면, 학점이 많은 쪽을 더 좋게 본다
    if (candidate->total_credit > best->total_credit) return 1;
    // 학점이 더 적으면 더 나쁘다
    if (candidate->total_credit < best->total_credit) return 0;

    // 학점도 같으면 평균 평점이 더 높은 쪽이 더 좋다
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
    // 지금까지의 선택이 (필수 과목 전부 포함 + 모드 조건 만족)이면 정답 후보가 된다
    if (has_all_required(cur) && meets_mode_condition(cur, mode)) {
        // 그 후보가 기존 best보다 더 좋으면
        if (is_better_schedule(cur, best, mode, *found_best)) {
            // 좋은 시간표를 찾았다고 표시하고
            *found_best = 1;
            // 현재 시간표를 best에 통째로 복사해 둔다
            *best = *cur;
        }
    }

    // 더 담을 자리가 없으면(최대 개수 도달) 여기서 멈춘다
    if (cur->count >= MAX_SELECTED) {
        return;
    }

    // start_idx부터 끝까지, 다음에 넣어 볼 강의 후보를 하나씩 본다
    // (start_idx부터 보기 때문에 이미 지나친 강의를 다시 넣지 않아 중복 조합을 막는다)
    for (int i = start_idx; i < course_count; i++) {
        // i번 강의의 주소를 가져온다
        Course *c = &course_list[i];

        // 이 강의를 넣으면 학점 한도를 넘는 경우 건너뛴다(불필요한 탐색 가지치기)
        if (cur->total_credit + c->credit > max_credit) continue;
        // 기존 강의들과 시간이 겹치면 넣을 수 없으니 건너뛴다(가지치기)
        if (!can_add(cur, i)) continue;

        // 강의를 하나 추가한 상태로 더 깊이 탐색한다.
        // 선택 목록 끝 칸에 i번 강의를 넣고,
        cur->indices[cur->count] = i;
        // 선택한 강의 개수를 1 늘리고,
        cur->count++;
        // 총 학점에 이 강의의 학점을 더한다
        cur->total_credit += c->credit;

        // 자기 자신을 다시 호출(재귀)해 i+1번 강의부터 이어서 탐색한다
        backtrack(cur, i + 1, mode, max_credit, best, found_best);

        // 재귀 호출이 끝나면 방금 넣은 강의를 다시 빼서 원래 상태로 되돌린다.
        // 그래야 다음 후보 강의를 넣어 보는 탐색을 계속할 수 있다.
        // 선택 개수를 도로 1 줄이고,
        cur->count--;
        // 더했던 학점도 도로 뺀다(되돌리기 완료)
        cur->total_credit -= c->credit;
    }
}

/* 세 최적화 모드가 공통으로 사용하는 실제 탐색 준비 함수입니다. */
static int run_optimizer(int max_credit, Schedule *result, int mode) {
    // 가장 좋은 시간표를 담아 둘 변수
    Schedule best;
    // 지금 만들어 보는 중인 시간표
    Schedule cur;
    // 아직 답을 하나도 못 찾았음을 뜻하는 표시(0)
    int found_best = 0;

    // memset은 구조체 안의 모든 바이트를 0으로 채워 깨끗이 초기화한다
    memset(&best, 0, sizeof(Schedule));
    // cur도 똑같이 0으로 초기화한다
    memset(&cur, 0, sizeof(Schedule));

    // 필수 과목은 반드시 들어가야 하므로 탐색을 시작하기 전에 cur에 먼저 담는다
    // 담은 필수 과목 개수를 세는 변수
    int required_count = 0;
    // 전체 강의를 하나씩 보며
    for (int i = 0; i < course_count; i++) {
        // 필수 과목이면
        if (course_list[i].is_required) {
            // 담을 자리가 없으면(최대 개수 초과) 만들 수 없으므로
            if (cur.count >= MAX_SELECTED) {
                // 경고를 보여 주고
                printf("\n[경고] 필수 과목이 너무 많습니다.\n");
                printf("   한 시간표에는 최대 %d개 강의만 담을 수 있습니다.\n",
                       MAX_SELECTED);
                // 실패(0)로 끝낸다
                return 0;
            }
            // 선택 목록 끝에 이 필수 과목을 넣고
            cur.indices[cur.count] = i;
            // 선택 개수를 1 늘리고
            cur.count++;
            // 총 학점에 그 학점을 더하고
            cur.total_credit += course_list[i].credit;
            // 담은 필수 과목 수를 1 늘린다
            required_count++;
        }
    }

    // 필수 과목끼리 시간이 겹치면 어떤 선택 과목을 골라도 해결할 수 없다
    // 충돌한 두 강의 번호를 받아 둘 변수
    int ci, cj;
    // 필수 과목들 사이에 시간 충돌이 있으면(check_conflict가 0을 돌려주면)
    if (!check_conflict(&cur, &ci, &cj)) {
        // 어떤 두 과목이 겹치는지 알려 주고
        printf("\n[경고] 필수 과목끼리 시간이 겹칩니다!\n");
        printf("   - %s  vs  %s\n", course_list[ci].name, course_list[cj].name);
        printf("   먼저 필수 과목 충돌을 해결해 주세요.\n");
        // 실패(0)로 끝낸다
        return 0;
    }

    // 필수 과목만으로 이미 학점 한도를 넘으면 더 만들 수 없다
    if (cur.total_credit > max_credit) {
        // 안내하고
        printf("\n[경고] 필수 과목만으로도 %d학점이라 한도(%d학점)를 넘습니다.\n",
               cur.total_credit, max_credit);
        // 실패(0)로 끝낸다
        return 0;
    }

    // 선택 과목 수 = 전체 강의 수 - 필수 과목 수
    int elective_count = course_count - required_count;
    // 선택 과목이 너무 많으면 탐색이 오래 걸릴 수 있다고 미리 알려 준다
    if (elective_count > SEARCH_WARN_THRESHOLD) {
        printf("\n  ※ 선택 과목이 %d개로 많아 탐색이 다소 오래 걸릴 수 있습니다.\n",
               elective_count);
    }

    // 0번 강의부터 시작해 가능한 조합을 모두 탐색한다(가장 좋은 시간표를 best에 모음)
    backtrack(&cur, 0, mode, max_credit, &best, &found_best);

    // 조건에 맞는 시간표를 하나도 못 찾았으면 실패(0)
    if (!found_best) return 0;

    // 찾은 best를 호출한 쪽이 준 result에 복사한다
    *result = best;
    // 결과의 총 학점과 평균 평점을 계산해 채워 넣는다
    compute_stats(result);
    // 성공(1)을 돌려준다
    return 1;
}

/* 아래 세 함수는 main.c에서 모드별로 호출하기 좋은 이름을 제공하는 래퍼입니다. */
// 공강 만들기(모드 1)로 최적화한다
int optimize_free_day(int max_credit, Schedule *result) {
    // 공통 탐색 함수에 모드 번호 1을 넘긴다
    return run_optimizer(max_credit, result, 1);
}

// 1교시 없애기(모드 2)로 최적화한다
int optimize_no_first_period(int max_credit, Schedule *result) {
    // 공통 탐색 함수에 모드 번호 2를 넘긴다
    return run_optimizer(max_credit, result, 2);
}

// 평점 우선(모드 3)으로 최적화한다
int optimize_high_rating(int max_credit, Schedule *result) {
    // 공통 탐색 함수에 모드 번호 3을 넘긴다
    return run_optimizer(max_credit, result, 3);
}
