// 헤더 중복 포함 방지: OPTIMIZER_H 가 아직 정의되지 않았을 때만 아래 내용을 포함한다
#ifndef OPTIMIZER_H
// OPTIMIZER_H 라는 이름표를 정의해, 이 파일이 두 번 포함되지 않도록 표시한다
#define OPTIMIZER_H

/*
 * optimizer.h
 * -----------
 * 시간표를 자동으로 고르는 최적화 함수들의 선언입니다.
 * 세 함수는 모두 최대 학점(max_credit)을 받고, 찾은 시간표를 result가 가리키는
 * Schedule 구조체 안에 채워 넣습니다.
 */

// 이 파일에서 쓰는 Schedule 구조체와 함수 약속이 담긴 헤더를 가져온다
#include "schedule.h"

// 공강 요일이 생기도록 시간표를 고른다(성공하면 1, 실패하면 0)
int optimize_free_day(int max_credit, Schedule *result);

// 1교시 수업이 없도록 시간표를 고른다(성공하면 1, 실패하면 0)
int optimize_no_first_period(int max_credit, Schedule *result);

// 평점이 높은 조합을 우선해서 시간표를 고른다(성공하면 1, 실패하면 0)
int optimize_high_rating(int max_credit, Schedule *result);

// #ifndef OPTIMIZER_H 로 시작한 중복 포함 방지 블록을 여기서 닫는다
#endif
