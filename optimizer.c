#include <stdio.h>
#include <string.h>
#include "optimizer.h"

#define SEARCH_WARN_THRESHOLD   25

typedef struct {
    double key[3];
    int    len;
} Score;

typedef Score (*ScoreFn)(const Schedule *);

typedef int   (*FilterFn)(const Schedule *);

static int score_better(const Score *a, const Score *b) {
    int n = (a->len < b->len) ? a->len : b->len;
    for (int i = 0; i < n; i++) {
        if (a->key[i] > b->key[i]) return 1;
        if (a->key[i] < b->key[i]) return 0;
    }
    return 0;
}

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

static Schedule g_best;
static Score    g_best_score;
static int      g_found;
static int      g_max_credit;
static ScoreFn  g_score_fn;
static FilterFn g_filter_fn;

static int has_all_required(const Schedule *s) {
    for (int i = 0; i < course_count; i++) {
        if (!course_list[i].is_required) continue;

        int found = 0;
        for (int j = 0; j < s->count; j++) {
            if (s->indices[j] == i) { found = 1; break; }
        }
        if (!found) return 0;
    }
    return 1;
}

static void backtrack(Schedule *cur, int start_idx) {

    if (has_all_required(cur) && g_filter_fn(cur)) {
        Score sc = g_score_fn(cur);
        if (!g_found || score_better(&sc, &g_best_score)) {
            g_found      = 1;
            g_best_score = sc;
            g_best       = *cur;
        }
    }

    if (cur->count >= MAX_SELECTED) {
        return;
    }

    for (int i = start_idx; i < course_count; i++) {
        Course *c = &course_list[i];

        if (cur->total_credit + c->credit > g_max_credit) continue;
        if (!can_add(cur, i)) continue;

        cur->indices[cur->count++] = i;
        cur->total_credit += c->credit;

        backtrack(cur, i + 1);

        cur->count--;
        cur->total_credit -= c->credit;
    }
}

static int run_optimizer(int max_credit, Schedule *result,
                         ScoreFn score_fn, FilterFn filter_fn) {
    g_found      = 0;
    g_max_credit = max_credit;
    g_score_fn   = score_fn;
    g_filter_fn  = filter_fn;
    memset(&g_best, 0, sizeof(Schedule));

    Schedule cur;
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
            cur.indices[cur.count++] = i;
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

    backtrack(&cur, 0);

    if (!g_found) return 0;

    *result = g_best;
    compute_stats(result);
    return 1;
}

static Score score_free_day(const Schedule *s) {
    Score sc = {{0}, 0};
    sc.len    = 3;
    sc.key[0] = count_free_days(s);
    sc.key[1] = s->total_credit;
    sc.key[2] = schedule_avg_rating(s);
    return sc;
}

static int filter_free_day(const Schedule *s) {
    return count_free_days(s) >= 1;
}

static Score score_credit_then_rating(const Schedule *s) {
    Score sc = {{0}, 0};
    sc.len    = 2;
    sc.key[0] = s->total_credit;
    sc.key[1] = schedule_avg_rating(s);
    return sc;
}

static int filter_no_first(const Schedule *s) {
    return !has_period_1(s);
}

static int filter_always(const Schedule *s) {
    (void)s;
    return 1;
}

int optimize_free_day(int max_credit, Schedule *result) {
    return run_optimizer(max_credit, result, score_free_day, filter_free_day);
}

int optimize_no_first_period(int max_credit, Schedule *result) {
    return run_optimizer(max_credit, result, score_credit_then_rating, filter_no_first);
}

int optimize_high_rating(int max_credit, Schedule *result) {
    return run_optimizer(max_credit, result, score_credit_then_rating, filter_always);
}
