#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "schedule.h"

int optimize_free_day(int max_credit, Schedule *result);
int optimize_no_first_period(int max_credit, Schedule *result);
int optimize_high_rating(int max_credit, Schedule *result);

#endif
