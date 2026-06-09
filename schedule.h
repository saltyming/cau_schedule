// 헤더 중복 포함 방지: SCHEDULE_H 가 아직 정의되지 않았을 때만 아래 내용을 포함한다
#ifndef SCHEDULE_H
// SCHEDULE_H 라는 이름표를 정의해, 이 파일이 두 번 포함되지 않도록 표시한다
#define SCHEDULE_H

/*
 * schedule.h
 * ----------
 * 하나의 시간표(Schedule)를 어떻게 저장하고 검사할지 정의합니다.
 * 강의 자체의 정보는 Course에 있고, Schedule은 "course_list 배열의 몇 번째 강의를
 * 선택했는지"만 indices 배열에 저장합니다.
 */

// 이 파일에서 쓰는 Course 구조체와 상수가 들어 있는 헤더를 가져온다
#include "course.h"

// 한 시간표에 담을 수 있는 최대 강의 수
#define MAX_SELECTED    30

/*
 * Schedule 구조체
 * - indices: 선택된 강의들의 번호를 저장하는 배열입니다.
 *   예를 들어 indices[0]이 3이면 course_list[3] 강의를 선택했다는 뜻입니다.
 * - count: indices 배열에 실제로 몇 개가 들어 있는지입니다.
 * - total_credit, avg_rating: 출력과 비교를 쉽게 하려고 계산해 두는 통계값입니다.
 */
// typedef struct { ... } Schedule; 는 이 묶음 자료형을 Schedule 이라 부르겠다는 뜻
typedef struct {
    // 선택한 강의들의 번호(course_list 의 인덱스)를 담는 배열
    int   indices[MAX_SELECTED];
    // indices 에 실제로 들어 있는 강의 개수
    int   count;
    // 선택한 강의들의 학점을 모두 더한 값(미리 계산해 둠)
    int   total_credit;
    // 선택한 강의들의 평균 평점(미리 계산해 둠)
    float avg_rating;
} Schedule;

/*
 * const Schedule *s 처럼 별표(*)가 붙은 매개변수는 구조체 전체를 복사하지 않고
 * 그 구조체가 있는 메모리 주소만 전달한다는 뜻입니다.
 * const는 이 함수가 s가 가리키는 Schedule 내용을 바꾸지 않겠다는 약속입니다.
 */
// 시간표 s 안에 시간이 겹치는 강의가 있는지 검사한다(겹치면 ci, cj 에 그 번호를 알려 줌)
int  check_conflict(const Schedule *s, int *ci, int *cj);
// 시간표에 1교시 수업이 하나라도 있으면 1을 돌려준다
int  has_period_1(const Schedule *s);
// 월~금 중 수업이 하나도 없는 요일(공강)의 수를 센다
int  count_free_days(const Schedule *s);
// course_idx 강의를 시간표 s 에 넣어도 시간 충돌이 없는지 검사한다
int  can_add(const Schedule *s, int course_idx);
// 특정 요일·교시 칸에 들어 있는 강의 번호를 찾는다(없으면 -1)
int  course_at(const Schedule *s, int day, int period);
// 시간표의 총 학점과 평균 평점을 계산해 Schedule 안에 채워 넣는다
void compute_stats(Schedule *s);

// #ifndef SCHEDULE_H 로 시작한 중복 포함 방지 블록을 여기서 닫는다
#endif
