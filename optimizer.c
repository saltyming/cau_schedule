#include <stdio.h>
#include <string.h>
#include "optimizer.h"

// 선택 과목이 이 수를 넘으면 모든 조합을 살펴보는 데 시간이 걸릴 수 있어
// 사용자에게 미리 알려 줍니다. (탐색은 가능한 조합을 모두 시도하는 방식입니다)
#define SEARCH_WARN_THRESHOLD   25

// MARK: - 시간표 점수 매기기
//
// 시간표 한 개를 '점수'로 나타내 서로 비교합니다. 점수는 숫자 하나가 아니라
// 우선순위가 있는 숫자 묶음입니다. 예) {공강요일, 학점, 평균평점}.
// 비교할 때는 앞쪽(중요한) 항목부터 봅니다. 앞 항목이 같을 때만 다음 항목을
// 봅니다. 이런 비교 방식을 '사전식(사전에서 단어를 비교하듯)'이라고 합니다.
//
// 모든 모드가 학점(채우기)을 평점보다 앞에 두기 때문에, 시간표는 먼저 한도까지
// 채워지고, 그렇게 채운 시간표들 사이에서 평점이 높은 쪽이 뽑힙니다.

/// 시간표 한 개의 점수입니다. key[0]이 가장 중요하고 뒤로 갈수록 덜 중요합니다.
typedef struct {
    double key[3];
    int    len;     // 실제로 사용하는 key 항목 수 (모드마다 다름)
} Score;

/// 시간표가 어떤 모드에서 얼마나 좋은지 점수를 계산하는 함수의 형태입니다.
typedef Score (*ScoreFn)(const Schedule *);

/// 시간표가 모드의 필수 조건(예: 1교시 없음)을 만족하는지 보는 함수의 형태입니다.
typedef int   (*FilterFn)(const Schedule *);

/**
 두 점수를 사전식으로 비교합니다.

 앞 항목(더 중요한 항목)부터 차례로 보며, 더 큰 쪽이 더 좋은 시간표입니다.

 - Parameter a: 비교할 점수
 - Parameter b: 기준이 되는 점수
 - Returns: a가 b보다 좋으면 1, 아니면 0.
 */
static int score_better(const Score *a, const Score *b) {
    int n = (a->len < b->len) ? a->len : b->len;
    for (int i = 0; i < n; i++) {
        if (a->key[i] > b->key[i]) return 1;
        if (a->key[i] < b->key[i]) return 0;
    }
    return 0;   // 모든 항목이 같으면 더 낫다고 보지 않습니다.
}

/**
 시간표에 담긴 강의들의 평점을 모두 더합니다. (평균을 구하는 데 씁니다)

 - Parameter s: 대상 시간표
 - Returns: 평점들의 합.
 */
static double schedule_rating_sum(const Schedule *s) {
    double sum = 0.0;
    for (int i = 0; i < s->count; i++) {
        sum += course_list[s->indices[i]].rating;
    }
    return sum;
}

/**
 시간표에 담긴 강의들의 평균 평점을 구합니다.

 평균을 시간표의 '품질' 지표로 씁니다. 시간표를 채우는 일은 학점을 먼저
 비교해 처리하므로, 평균은 학점이 같은 시간표들 사이에서 더 좋은 쪽을
 고르는 기준이 됩니다. (그래서 평균을 써도 시간표가 비지 않습니다. 합을
 쓰면 평점이 낮아도 강의 수가 많은 쪽을 골라 버리는 문제가 있습니다.)

 - Parameter s: 대상 시간표
 - Returns: 평균 평점. 담긴 강의가 없으면 0.
 */
static double schedule_avg_rating(const Schedule *s) {
    return (s->count > 0) ? schedule_rating_sum(s) / s->count : 0.0;
}

// MARK: - 백트래킹 탐색 엔진
//
// '백트래킹'은 강의를 하나씩 넣어 보고, 막다른 길이면 도로 빼는 식으로 가능한
// 조합을 빠짐없이 시도하는 방법입니다. 그 과정에서 가장 점수가 높은 시간표를
// 기억해 둡니다. 아래 전역 변수들은 탐색하는 동안 공유하는 작업 공간입니다.

static Schedule g_best;         // 지금까지 찾은 가장 좋은 시간표
static Score    g_best_score;   // 그 시간표의 점수
static int      g_found;        // 조건을 만족하는 시간표를 하나라도 찾았는지(0/1)
static int      g_max_credit;   // 학점 한도
static ScoreFn  g_score_fn;     // 현재 모드의 점수 계산 함수
static FilterFn g_filter_fn;    // 현재 모드의 필수 조건 함수

/**
 시간표가 모든 필수 과목을 포함하고 있는지 확인합니다.

 - Parameter s: 검사할 시간표
 - Returns: 필수 과목을 모두 담았으면 1, 하나라도 빠졌으면 0.
 */
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

/**
 가능한 강의 조합을 재귀적으로 탐색하며 가장 좋은 시간표를 찾습니다.

 - Parameter cur: 지금까지 담은 시간표(탐색 도중 계속 바뀝니다)
 - Parameter start_idx: 이번 단계에서 새로 살펴보기 시작할 강의 번호
 */
static void backtrack(Schedule *cur, int start_idx) {
    // 지금 시간표가 '필수 과목 모두 포함 + 모드 조건 만족'이면 점수를 매겨,
    // 지금까지의 최고 기록보다 좋으면 갈아 끼웁니다.
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

    // 남은 강의들을 하나씩 더 넣어 보며 더 좋은 조합을 찾습니다.
    for (int i = start_idx; i < course_count; i++) {
        Course *c = &course_list[i];

        if (cur->total_credit + c->credit > g_max_credit) continue;  // 학점 한도 초과
        if (!can_add(cur, i)) continue;                              // 시간 겹침(이미 담은 강의 포함)

        // 넣어 보기
        cur->indices[cur->count++] = i;
        cur->total_credit += c->credit;

        backtrack(cur, i + 1);

        // 도로 빼기(백트래킹)
        cur->count--;
        cur->total_credit -= c->credit;
    }
}

/**
 공통 탐색 절차를 실행합니다. 모드별로 점수/필수조건 함수만 바꿔 끼웁니다.

 - Parameter max_credit: 학점 한도
 - Parameter result: 찾은 시간표를 담을 곳
 - Parameter score_fn: 이 모드의 점수 계산 함수
 - Parameter filter_fn: 이 모드의 필수 조건 함수
 - Returns: 시간표를 찾으면 1, 못 찾으면 0.
 */
static int run_optimizer(int max_credit, Schedule *result,
                         ScoreFn score_fn, FilterFn filter_fn) {
    g_found      = 0;
    g_max_credit = max_credit;
    g_score_fn   = score_fn;
    g_filter_fn  = filter_fn;
    memset(&g_best, 0, sizeof(Schedule));

    Schedule cur;
    memset(&cur, 0, sizeof(Schedule));

    // 필수 과목은 무조건 들어가야 하므로 먼저 모두 담아 둡니다.
    int required_count = 0;
    for (int i = 0; i < course_count; i++) {
        if (course_list[i].is_required) {
            if (cur.count >= MAX_SELECTED) {
                printf("\n⚠ 필수 과목이 너무 많습니다.\n");
                printf("   한 시간표에는 최대 %d개 강의만 담을 수 있습니다.\n",
                       MAX_SELECTED);
                return 0;
            }
            cur.indices[cur.count++] = i;
            cur.total_credit += course_list[i].credit;
            required_count++;
        }
    }

    // 필수 과목끼리 시간이 겹치면 어떤 시간표도 만들 수 없습니다.
    int ci, cj;
    if (!check_conflict(&cur, &ci, &cj)) {
        printf("\n⚠ 필수 과목끼리 시간이 겹칩니다!\n");
        printf("   - %s  vs  %s\n", course_list[ci].name, course_list[cj].name);
        printf("   먼저 필수 과목 충돌을 해결해 주세요.\n");
        return 0;
    }

    // 필수 과목만으로 이미 학점 한도를 넘으면 만들 수 없습니다.
    if (cur.total_credit > max_credit) {
        printf("\n⚠ 필수 과목만으로도 %d학점이라 한도(%d학점)를 넘습니다.\n",
               cur.total_credit, max_credit);
        return 0;
    }

    // 선택 과목이 너무 많으면 모든 조합을 살펴보는 데 시간이 걸릴 수 있습니다.
    int elective_count = course_count - required_count;
    if (elective_count > SEARCH_WARN_THRESHOLD) {
        printf("\n  ※ 선택 과목이 %d개로 많아 탐색이 다소 오래 걸릴 수 있습니다.\n",
               elective_count);
    }

    backtrack(&cur, 0);

    if (!g_found) return 0;   // 조건을 만족하는 시간표를 끝내 찾지 못함

    *result = g_best;
    compute_stats(result);    // 화면·파일에 보여 줄 요약값(총학점/평균평점) 계산
    return 1;
}

// MARK: - 모드별 점수와 필수 조건

// 모드 1 — 공강 하루 만들기
// 우선순위: 공강 요일 → 학점(채우기) → 평균 평점
// 먼저 쉬는 날을 만들고, 그 안에서 한도까지 채운 뒤, 평점으로 동점을 가립니다.

/// 모드 1의 점수: {공강 요일 수, 총 학점, 평균 평점}
static Score score_free_day(const Schedule *s) {
    Score sc = {{0}, 0};
    sc.len    = 3;
    sc.key[0] = count_free_days(s);
    sc.key[1] = s->total_credit;
    sc.key[2] = schedule_avg_rating(s);
    return sc;
}

/// 모드 1의 필수 조건: 공강 요일이 하루 이상 있을 것
static int filter_free_day(const Schedule *s) {
    return count_free_days(s) >= 1;
}

// 모드 2·3 공통 점수
// 우선순위: 학점(채우기) → 평균 평점
// "한도까지 채운 다음, 그중 평점이 가장 높은 시간표"를 고릅니다. 두 모드의
// 차이는 점수가 아니라 조건입니다(모드 2는 1교시 금지, 모드 3은 제한 없음).
// 그래서 모드 3은 평점 높은 1교시 강의까지 자유롭게 담아 더 높은 평점을 낼 수
// 있고, 모드 2는 1교시를 피하면서 가능한 한 좋은 시간표를 만듭니다.

/// 모드 2·3의 점수: {총 학점, 평균 평점}
static Score score_credit_then_rating(const Schedule *s) {
    Score sc = {{0}, 0};
    sc.len    = 2;
    sc.key[0] = s->total_credit;
    sc.key[1] = schedule_avg_rating(s);
    return sc;
}

/// 모드 2의 필수 조건: 1교시 수업이 없을 것
static int filter_no_first(const Schedule *s) {
    return !has_period_1(s);
}

/// 모드 3의 필수 조건: 없음(공통 규칙만 지키면 됨)
static int filter_always(const Schedule *s) {
    (void)s;   // 사용하지 않는 매개변수 경고를 막습니다.
    return 1;
}

// MARK: - 공개 함수 (세 가지 모드)

int optimize_free_day(int max_credit, Schedule *result) {
    return run_optimizer(max_credit, result, score_free_day, filter_free_day);
}

int optimize_no_first_period(int max_credit, Schedule *result) {
    return run_optimizer(max_credit, result, score_credit_then_rating, filter_no_first);
}

int optimize_high_rating(int max_credit, Schedule *result) {
    return run_optimizer(max_credit, result, score_credit_then_rating, filter_always);
}
