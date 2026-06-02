#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "schedule.h"

// MARK: - 세 가지 시간표 자동 생성
//
// 아래 세 함수는 모두 다음 공통 규칙을 지킵니다.
//   · 필수 과목은 반드시 모두 포함합니다.
//   · 강의끼리 시간이 겹치지 않습니다.
//   · 총 학점이 max_credit(한도)을 넘지 않습니다.
//   · 한도를 '목표'로 보고, 한도 안에서 최대한 채웁니다.
// 세 함수의 차이는 "무엇을 가장 우선해서 좋게 만드는가"입니다.

/**
 모드 1 — 공강(쉬는 날)이 하루 이상 있는 시간표를 만듭니다.

 공강 요일 수를 가장 우선해서 늘리고, 그다음 학점을 채우며,
 조건이 같으면 평점이 높은 쪽을 고릅니다.

 - Parameter max_credit: 들을 수 있는 최대 학점
 - Parameter result: 만들어진 시간표를 담을 곳
 - Returns: 조건을 만족하는 시간표를 찾으면 1, 못 찾으면 0.
 */
int optimize_free_day(int max_credit, Schedule *result);

/**
 모드 2 — 1교시 수업이 하나도 없는 시간표를 만듭니다.

 한도까지 학점을 채우되, 조건이 같으면 평점이 높은 쪽을 고릅니다.

 - Parameter max_credit: 들을 수 있는 최대 학점
 - Parameter result: 만들어진 시간표를 담을 곳
 - Returns: 조건을 만족하는 시간표를 찾으면 1, 못 찾으면 0.
 */
int optimize_no_first_period(int max_credit, Schedule *result);

/**
 모드 3 — 평점이 높은 강의 위주로 채운 시간표를 만듭니다.

 한도까지 채운 시간표들 중에서 평균 평점이 가장 높은 것을 고릅니다.
 모드 2와 목표는 같지만, 1교시 제한이 없어 평점 높은 1교시 강의까지
 자유롭게 담을 수 있다는 점이 다릅니다.

 - Parameter max_credit: 들을 수 있는 최대 학점
 - Parameter result: 만들어진 시간표를 담을 곳
 - Returns: 조건을 만족하는 시간표를 찾으면 1, 못 찾으면 0.
 */
int optimize_high_rating(int max_credit, Schedule *result);

#endif
