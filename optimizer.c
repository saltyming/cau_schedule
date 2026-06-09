#include <stdio.h>
#include <string.h>
#include "optimizer.h"

#define SEARCH_WARN_THRESHOLD   25

static double schedule_rating_sum(const Schedule *s) {
    double sum = 0.0;
    for (int i = 0; i < s->count; i++) {
        sum += course_list[s->indices[i]].rating;
    }
    return sum;
}

static double schedule_avg_rating(const Schedule *s) {
    return (s->count > 0) ? schedule_rating_sum(s) / s->count : 0.0;
}

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

static int meets_mode_condition(const Schedule *s, int mode) {
    if (mode == 1) {
        return count_free_days(s) >= 1;
    }
    if (mode == 2) {
        return !has_period_1(s);
    }
    return 1;
}

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

static void backtrack(Schedule *cur,
                      int start_idx,
                      int mode,
                      int max_credit,
                      Schedule *best,
                      int *found_best) {
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

        cur->indices[cur->count] = i;
        cur->count++;
        cur->total_credit += c->credit;

        backtrack(cur, i + 1, mode, max_credit, best, found_best);

        cur->count--;
        cur->total_credit -= c->credit;
    }
}

static int run_optimizer(int max_credit, Schedule *result, int mode) {
    Schedule best;
    Schedule cur;
    int found_best = 0;

    memset(&best, 0, sizeof(Schedule));
    memset(&cur, 0, sizeof(Schedule));

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

int optimize_free_day(int max_credit, Schedule *result) {
    return run_optimizer(max_credit, result, 1);
}

int optimize_no_first_period(int max_credit, Schedule *result) {
    return run_optimizer(max_credit, result, 2);
}

int optimize_high_rating(int max_credit, Schedule *result) {
    return run_optimizer(max_credit, result, 3);
}
