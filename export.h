// 헤더 중복 포함 방지: EXPORT_H 가 아직 정의되지 않았을 때만 아래 내용을 포함한다
#ifndef EXPORT_H
// EXPORT_H 라는 이름표를 정의해, 이 파일이 두 번 포함되지 않도록 표시한다
#define EXPORT_H

/*
 * export.h
 * --------
 * 완성된 시간표를 CSV 파일로 저장하는 함수의 선언입니다.
 */

// 이 파일에서 쓰는 Schedule 구조체와 함수 약속이 담긴 헤더를 가져온다
#include "schedule.h"

/*
 * path: 저장할 파일 이름 문자열입니다.
 * titles[]: 각 시간표에 붙일 제목 문자열 배열입니다.
 * schedules[]: 저장할 Schedule 구조체 배열입니다.
 * count: 배열에 실제로 몇 개의 시간표가 들어 있는지입니다.
 */
// 여러 시간표를 한 CSV 파일로 저장한다(성공하면 1, 실패하면 0)
int save_schedules_csv(const char *path, const char *titles[],
                       const Schedule schedules[], int count);

// #ifndef EXPORT_H 로 시작한 중복 포함 방지 블록을 여기서 닫는다
#endif
