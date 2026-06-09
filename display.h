#ifndef DISPLAY_H
#define DISPLAY_H

/*
 * display.h
 * ---------
 * 완성된 시간표를 콘솔 화면에 보여 주는 함수의 선언입니다.
 */

#include "schedule.h"

/*
 * s는 출력할 시간표의 주소이고, title은 화면에 표시할 제목 문자열의 주소입니다.
 * const가 붙었으므로 display_summary는 두 값을 읽기만 하고 수정하지 않습니다.
 */
void display_summary(const Schedule *s, const char *title);

#endif
