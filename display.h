// 헤더 중복 포함 방지: DISPLAY_H 가 아직 정의되지 않았을 때만 아래 내용을 포함한다
#ifndef DISPLAY_H
// DISPLAY_H 라는 이름표를 정의해, 이 파일이 두 번 포함되지 않도록 표시한다
#define DISPLAY_H

/*
 * display.h
 * ---------
 * 완성된 시간표를 콘솔 화면에 보여 주는 함수의 선언입니다.
 */

// 이 파일에서 쓰는 Schedule 구조체와 함수 약속이 담긴 헤더를 가져온다
#include "schedule.h"

/*
 * s는 출력할 시간표의 주소이고, title은 화면에 표시할 제목 문자열의 주소입니다.
 * const가 붙었으므로 display_summary는 두 값을 읽기만 하고 수정하지 않습니다.
 */
// 시간표 s 를 제목 title 과 함께 화면에 보기 좋게 출력한다
void display_summary(const Schedule *s, const char *title);

// #ifndef DISPLAY_H 로 시작한 중복 포함 방지 블록을 여기서 닫는다
#endif
