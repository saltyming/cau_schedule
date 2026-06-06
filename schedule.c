#include <stdio.h>
#include <string.h>
#include "schedule.h"

static int courses_overlap(int a, int b) {
    Course *ca = &course_list[a];
    Course *cb = &course_list[b];

    for (int da = 0; da < ca->day_count; da++) {
        for (int db = 0; db < cb->day_count; db++) {
            if (ca->days[da] != cb->days[db]) continue;

            if (ca->start_period <= cb->end_period &&
                cb->start_period <= ca->end_period) {
                return 1;
            }
        }
    }
    return 0;
}

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

int can_add(const Schedule *s, int course_idx) {
    for (int i = 0; i < s->count; i++) {
        if (courses_overlap(s->indices[i], course_idx)) return 0;
    }
    return 1;
}

int has_period_1(const Schedule *s) {
    for (int i = 0; i < s->count; i++) {
        if (course_list[s->indices[i]].start_period == 1) return 1;
    }
    return 0;
}

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
