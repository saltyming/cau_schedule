#ifndef EXPORT_H
#define EXPORT_H

#include "schedule.h"

// MARK: - 시간표를 CSV 파일로 저장

/**
 만들어진 시간표들을 하나의 CSV 파일로 저장합니다.

 한 파일에 여러 시간표를 '구역(섹션)'으로 나눠 담습니다. 각 구역은
 요일(가로) × 교시(세로) 표와 요약 정보, 강의 목록으로 이루어집니다.
 파일 맨 앞에는 UTF-8 표식(BOM)을 넣어, Excel에서 더블클릭으로 열어도
 한글이 깨지지 않게 합니다.

 - Parameter path: 저장할 파일 경로 (예: "result_schedules.csv")
 - Parameter titles: 각 시간표의 제목 배열
 - Parameter schedules: 저장할 시간표 배열
 - Parameter count: 저장할 시간표 개수
 - Returns: 저장에 성공하면 1, 파일을 열 수 없으면 0.
 */
int save_schedules_csv(const char *path, const char *titles[],
                       const Schedule schedules[], int count);

#endif
