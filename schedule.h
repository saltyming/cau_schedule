#ifndef SCHEDULE_H
#define SCHEDULE_H

/*
 * schedule.h
 * ----------
 * 하나의 시간표(Schedule)를 어떻게 저장하고 검사할지 정의합니다.
 * 강의 자체의 정보는 Course에 있고, Schedule은 "course_list 배열의 몇 번째 강의를
 * 선택했는지"만 indices 배열에 저장합니다.
 */

#include "course.h"

#define MAX_SELECTED    30  /* 한 시간표에 담을 수 있는 최대 강의 수 */

/*
 * Schedule 구조체
 * - indices: 선택된 강의들의 번호를 저장하는 배열입니다.
 *   예를 들어 indices[0]이 3이면 course_list[3] 강의를 선택했다는 뜻입니다.
 * - count: indices 배열에 실제로 몇 개가 들어 있는지입니다.
 * - total_credit, avg_rating: 출력과 비교를 쉽게 하려고 계산해 두는 통계값입니다.
 */
typedef struct {
    int   indices[MAX_SELECTED];
    int   count;
    int   total_credit;
    float avg_rating;
} Schedule;

/*
 * const Schedule *s 처럼 별표(*)가 붙은 매개변수는 구조체 전체를 복사하지 않고
 * 그 구조체가 있는 메모리 주소만 전달한다는 뜻입니다.
 * const는 이 함수가 s가 가리키는 Schedule 내용을 바꾸지 않겠다는 약속입니다.
 */
int  check_conflict(const Schedule *s, int *ci, int *cj);
int  has_period_1(const Schedule *s);
int  count_free_days(const Schedule *s);
int  can_add(const Schedule *s, int course_idx);
int  course_at(const Schedule *s, int day, int period);
void compute_stats(Schedule *s);

#endif
