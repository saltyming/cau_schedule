#ifndef DISPLAY_H
#define DISPLAY_H

#include "schedule.h"

// MARK: - 시간표 요약 출력

/**
 시간표의 요약(총 학점·평균 평점·공강 요일·1교시 여부·강의 목록)을
 테두리 없이 간단한 목록 형태로 화면에 출력합니다.

 - Parameter s: 요약할 시간표
 - Parameter title: 요약 위에 표시할 제목
 */
void display_summary(const Schedule *s, const char *title);

#endif
