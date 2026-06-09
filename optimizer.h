#ifndef OPTIMIZER_H
#define OPTIMIZER_H

/*
 * optimizer.h
 * -----------
 * 시간표를 자동으로 고르는 최적화 함수들의 선언입니다.
 * 세 함수는 모두 최대 학점(max_credit)을 받고, 찾은 시간표를 result가 가리키는
 * Schedule 구조체 안에 채워 넣습니다.
 */

#include "schedule.h"

/* 공강 요일이 생기도록 시간표를 고릅니다. */
int optimize_free_day(int max_credit, Schedule *result);

/* 1교시 수업이 없도록 시간표를 고릅니다. */
int optimize_no_first_period(int max_credit, Schedule *result);

/* 평점이 높은 조합을 우선해서 시간표를 고릅니다. */
int optimize_high_rating(int max_credit, Schedule *result);

#endif
