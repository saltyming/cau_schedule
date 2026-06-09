// 파일 출력(fopen, fputc 등)을 위한 표준 입출력 라이브러리
#include <stdio.h>
// 이 파일에서 쓰는 save_schedules_csv 약속과 Schedule 구조체가 담긴 헤더
#include "export.h"

/*
 * export.c
 * --------
 * 시간표 결과를 CSV 파일로 저장합니다. CSV는 쉼표로 칸을 구분하는 텍스트 파일이라
 * Excel 같은 프로그램에서 쉽게 열 수 있습니다.
 */

// 요일 번호 0~4를 파일에 쓸 한글 문자열로 바꾸기 위한 배열
static const char *DAY_NAMES[] = {"월", "화", "수", "목", "금"};

/*
 * CSV 한 칸을 안전하게 씁니다.
 * 값 안에 쉼표, 큰따옴표, 줄바꿈이 있으면 큰따옴표로 감싸야 CSV 형식이 깨지지 않습니다.
 */
static void write_csv_field(FILE *fp, const char *s) {
    // 큰따옴표로 감싸야 하는지 표시(처음엔 아니오=0)
    int needs_quote = 0;
    // 문자열을 한 글자씩 살펴본다(p 가 글자를 가리키며 한 칸씩 전진)
    for (const char *p = s; *p; p++) {
        // 쉼표·큰따옴표·줄바꿈 중 하나라도 있으면 그냥 쓰면 CSV가 깨진다
        if (*p == ',' || *p == '"' || *p == '\n' || *p == '\r') {
            // 감싸야 한다고 표시하고
            needs_quote = 1;
            // 더 볼 필요 없으니 멈춘다
            break;
        }
    }

    // 특수문자가 하나도 없으면
    if (!needs_quote) {
        // 값을 그대로 파일에 쓰고
        fputs(s, fp);
        // 끝낸다
        return;
    }

    // 특수문자가 있으면, 먼저 여는 큰따옴표를 쓴다
    fputc('"', fp);
    // 글자를 하나씩 보며
    for (const char *p = s; *p; p++) {
        // 글자가 큰따옴표면 CSV 규칙대로 한 번 더 써서 두 개("")로 만든다
        if (*p == '"') fputc('"', fp);
        // 원래 글자를 쓴다
        fputc(*p, fp);
    }
    // 마지막에 닫는 큰따옴표를 쓴다
    fputc('"', fp);
}

/* 시간표 표를 어디까지 출력할지 정하기 위해 가장 늦게 끝나는 교시를 찾습니다. */
static int last_used_period(const Schedule *s) {
    // 최소 1교시까지는 그린다(가장 늦은 교시를 1에서부터 갱신)
    int last = 1;
    // 선택된 강의들을 하나씩 보며
    for (int i = 0; i < s->count; i++) {
        // 그 강의의 종료 교시를 꺼낸다
        int end = course_list[s->indices[i]].end_period;
        // 지금까지 본 것보다 더 늦으면 그 값으로 갱신한다
        if (end > last) last = end;
    }
    // 가장 늦게 끝나는 교시를 돌려준다
    return last;
}

/* 요일을 열로, 교시를 행으로 하는 시간표 격자를 CSV에 씁니다. */
static void write_grid(FILE *fp, const Schedule *s) {

    // 표의 첫 칸에 "교시"라는 제목을 쓴다
    fputs("교시", fp);
    // 월~금 요일 이름을 제목 줄에 차례로 쓴다
    for (int d = 0; d < DAYS_IN_WEEK; d++) {
        // 칸을 나누는 쉼표를 먼저 쓰고
        fputc(',', fp);
        // 요일 이름을 쓴다
        fputs(DAY_NAMES[d], fp);
    }
    // 제목 줄을 끝내고 줄을 바꾼다
    fputc('\n', fp);

    // 표를 몇 교시까지 그릴지(가장 늦은 교시) 구한다
    int last = last_used_period(s);
    // 1교시부터 마지막 교시까지 한 줄(행)씩 쓴다
    for (int p = 1; p <= last; p++) {
        // 행의 맨 앞에 "p교시" 표시를 쓴다
        fprintf(fp, "%d교시", p);
        // 그 교시에 대해 월~금 칸을 차례로 채운다
        for (int d = 0; d < DAYS_IN_WEEK; d++) {
            // 칸을 나누는 쉼표를 먼저 쓴다
            fputc(',', fp);
            // 그 요일·교시 칸에 들어 있는 강의 번호를 찾는다(없으면 -1)
            int idx = course_at(s, d, p);
            // 강의가 있으면
            if (idx >= 0) {
                // 그 강의명을 안전하게 쓴다(특수문자 처리 포함)
                write_csv_field(fp, course_list[idx].name);
            }
        }
        // 한 교시 줄을 끝내고 줄을 바꾼다
        fputc('\n', fp);
    }
}

/* 총 학점, 평균 평점, 공강 요일, 강의 목록 같은 요약 정보를 CSV에 씁니다. */
static void write_summary(FILE *fp, const Schedule *s) {
    // "총 학점,숫자" 형태로 쓴다
    fprintf(fp, "총 학점,%d\n", s->total_credit);
    // "평균 평점,숫자" 형태로 쓴다(소수 둘째 자리까지)
    fprintf(fp, "평균 평점,%.2f\n", s->avg_rating);

    // used[d]가 1이면 d번째 요일에 수업이 있다는 뜻(처음엔 모두 0)
    int used[DAYS_IN_WEEK] = {0};
    // 선택된 강의들을 하나씩 보며
    for (int i = 0; i < s->count; i++) {
        // i번째로 선택한 강의의 주소를 가져온다
        Course *c = &course_list[s->indices[i]];
        // 그 강의가 수업하는 요일들을 하나씩 보며
        for (int d = 0; d < c->day_count; d++) {
            // 올바른 요일 번호면 그 요일을 '수업 있음'(1)으로 표시한다
            if (c->days[d] >= 0) used[c->days[d]] = 1;
        }
    }

    // "공강 요일," 까지 쓴 뒤, 비는 요일들을 이어서 쓴다
    fputs("공강 요일,", fp);
    // 공강이 하나라도 있었는지 표시(처음엔 없음=0)
    int any_free = 0;
    // 월~금을 하나씩 보며
    for (int d = 0; d < DAYS_IN_WEEK; d++) {
        // 수업이 없는 요일이면
        if (!used[d]) {
            // 그 요일 이름을 쓰고
            fputs(DAY_NAMES[d], fp);
            // 공강이 있었다고 표시한다
            any_free = 1;
        }
    }
    // 공강이 하나도 없으면 "없음"이라고 쓴다
    if (!any_free) fputs("없음", fp);
    // 공강 요일 줄을 끝내고 줄을 바꾼다
    fputc('\n', fp);

    // 1교시 수업이 있는지 "있음/없음"으로 쓴다
    fprintf(fp, "1교시 수업,%s\n", has_period_1(s) ? "있음" : "없음");

    // 강의 상세 목록의 제목 줄을 쓴다(앞의 빈 줄로 표와 구분)
    fputs("\n강의명,교수명,요일,시작교시,종료교시,학점,평점,필수\n", fp);
    // 선택된 강의들을 하나씩 보며
    for (int i = 0; i < s->count; i++) {
        // i번째로 선택한 강의의 주소를 가져온다
        Course *c = &course_list[s->indices[i]];

        // 강의명을 안전하게 쓴다
        write_csv_field(fp, c->name);
        // 칸 구분 쉼표
        fputc(',', fp);
        // 교수명을 안전하게 쓴다
        write_csv_field(fp, c->professor);
        // 칸 구분 쉼표
        fputc(',', fp);
        // 요일 글자들을 이어서 쓴다(예: 월수금)
        for (int d = 0; d < c->day_count; d++) {
            fputs(DAY_NAMES[c->days[d]], fp);
        }
        // 나머지 정보(시작·종료 교시, 학점, 평점, 필수 여부)를 한 줄로 쓴다
        fprintf(fp, ",%d,%d,%d,%.1f,%s\n",
                c->start_period, c->end_period, c->credit, c->rating,
                c->is_required ? "필수" : "선택");
    }
}

/*
 * 여러 개의 시간표를 하나의 CSV 파일에 저장합니다.
 * 성공하면 1, 파일 열기에 실패하면 0을 반환합니다.
 */
int save_schedules_csv(const char *path, const char *titles[],
                       const Schedule schedules[], int count) {
    // 파일을 쓰기 모드("w")로 연다. 같은 이름 파일이 있으면 새로 덮어쓴다
    FILE *fp = fopen(path, "w");
    // 파일을 열지 못했으면(권한 문제 등)
    if (!fp) {
        // 안내하고 실패(0)를 돌려준다
        printf("결과 파일을 저장할 수 없습니다: %s\n", path);
        return 0;
    }

    // Excel이 한글 UTF-8 CSV를 잘 인식하도록 맨 앞에 BOM 3바이트를 쓴다
    fputc(0xEF, fp);
    fputc(0xBB, fp);
    fputc(0xBF, fp);

    // 저장할 시간표 개수만큼 반복한다
    for (int i = 0; i < count; i++) {
        // 이 시간표의 제목을 쓰고
        write_csv_field(fp, titles[i]);
        // 줄을 바꾼다
        fputc('\n', fp);

        // 시간표 격자(요일×교시 표)를 쓰고
        write_grid(fp, &schedules[i]);
        // 표와 요약 사이에 빈 줄을 둔다
        fputc('\n', fp);

        // 요약 정보(학점·평점·공강·강의 목록)를 쓴다
        write_summary(fp, &schedules[i]);

        // 마지막 시간표가 아니면 다음 시간표와 빈 줄 두 개로 구분한다
        if (i < count - 1) fputs("\n\n", fp);
    }

    // 파일을 닫는다(열었으면 반드시 닫아 준다)
    fclose(fp);
    // 성공(1)을 돌려준다
    return 1;
}
