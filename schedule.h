#ifndef SCHEDULE_H
#define SCHEDULE_H

#include "course.h"

#define MAX_SELECTED    30

typedef struct {
    int   indices[MAX_SELECTED];
    int   count;
    int   total_credit;
    float avg_rating;
} Schedule;

int  check_conflict(const Schedule *s, int *ci, int *cj);
int  has_period_1(const Schedule *s);
int  count_free_days(const Schedule *s);
int  can_add(const Schedule *s, int course_idx);
int  course_at(const Schedule *s, int day, int period);
void compute_stats(Schedule *s);

#endif
