#ifndef SCHEDULE_H
#define SCHEDULE_H

#include "course.h"

// MARK: - 시간표 한 개를 표현하는 자료형

/// 한 시간표에 담을 수 있는 최대 강의 수입니다.
#define MAX_SELECTED    30

/**
 완성된 시간표 한 개를 표현하는 구조체입니다.

 어떤 강의를 담았는지(indices)와, 한눈에 보기 좋은 요약값
 (총 학점·평균 평점)을 함께 보관합니다.
 */
typedef struct {
    /// 담은 강의들의 course_list 인덱스 목록입니다.
    int   indices[MAX_SELECTED];
    /// indices에 담긴 강의 수입니다.
    int   count;
    /// 담은 강의들의 학점 합계입니다.
    int   total_credit;
    /// 담은 강의들의 평균 평점입니다.
    float avg_rating;
} Schedule;

// MARK: - 시간표 검사 / 계산 함수

/**
 시간표 안에 시간이 겹치는 강의 쌍이 있는지 검사합니다.

 - Parameter s: 검사할 시간표
 - Parameter ci: (결과 전달용) 겹치는 첫 번째 강의의 course_list 인덱스
 - Parameter cj: (결과 전달용) 겹치는 두 번째 강의의 course_list 인덱스
 - Returns: 겹침이 없으면 1, 있으면 0 (이때 ci/cj에 겹친 강의를 담아 줌).
 */
int  check_conflict(const Schedule *s, int *ci, int *cj);

/**
 시간표에 1교시 수업이 하나라도 있는지 확인합니다.

 - Parameter s: 검사할 시간표
 - Returns: 1교시 수업이 있으면 1, 없으면 0.
 */
int  has_period_1(const Schedule *s);

/**
 시간표에서 수업이 하나도 없는 '공강 요일'의 수를 셉니다.

 - Parameter s: 검사할 시간표
 - Returns: 월~금 중 수업이 전혀 없는 요일의 수.
 */
int  count_free_days(const Schedule *s);

/**
 어떤 강의를 시간표에 더해도 다른 강의와 시간이 겹치지 않는지 확인합니다.

 - Parameter s: 대상 시간표
 - Parameter course_idx: 추가하려는 강의의 course_list 인덱스
 - Returns: 겹치지 않아 추가할 수 있으면 1, 아니면 0.
 */
int  can_add(const Schedule *s, int course_idx);

/**
 특정 요일·교시 칸을 차지하고 있는 강의를 찾습니다.

 시간표를 표(요일 × 교시) 형태로 그릴 때, 각 칸에 어떤 강의가 들어가는지
 알아내는 데 사용합니다. 화면 출력과 파일 저장이 같은 함수를 공유합니다.

 - Parameter s: 대상 시간표
 - Parameter day: 요일 인덱스 (0=월 … 4=금)
 - Parameter period: 교시 번호 (1~9)
 - Returns: 그 칸을 차지한 강의의 course_list 인덱스. 비어 있으면 -1.
 */
int  course_at(const Schedule *s, int day, int period);

/**
 시간표의 요약값(총 학점·평균 평점)을 계산해 채워 넣습니다.

 - Parameter s: 요약값을 갱신할 시간표
 */
void compute_stats(Schedule *s);

#endif
