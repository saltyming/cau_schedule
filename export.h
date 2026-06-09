#ifndef EXPORT_H
#define EXPORT_H

/*
 * export.h
 * --------
 * 완성된 시간표를 CSV 파일로 저장하는 함수의 선언입니다.
 */

#include "schedule.h"

/*
 * path: 저장할 파일 이름 문자열입니다.
 * titles[]: 각 시간표에 붙일 제목 문자열 배열입니다.
 * schedules[]: 저장할 Schedule 구조체 배열입니다.
 * count: 배열에 실제로 몇 개의 시간표가 들어 있는지입니다.
 */
int save_schedules_csv(const char *path, const char *titles[],
                       const Schedule schedules[], int count);

#endif
