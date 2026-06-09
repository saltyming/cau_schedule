#include <stdio.h>
#include "export.h"

/*
 * export.c
 * --------
 * 시간표 결과를 CSV 파일로 저장합니다. CSV는 쉼표로 칸을 구분하는 텍스트 파일이라
 * Excel 같은 프로그램에서 쉽게 열 수 있습니다.
 */

static const char *DAY_NAMES[] = {"월", "화", "수", "목", "금"};

/*
 * CSV 한 칸을 안전하게 씁니다.
 * 값 안에 쉼표, 큰따옴표, 줄바꿈이 있으면 큰따옴표로 감싸야 CSV 형식이 깨지지 않습니다.
 */
static void write_csv_field(FILE *fp, const char *s) {
    int needs_quote = 0;
    for (const char *p = s; *p; p++) {
        if (*p == ',' || *p == '"' || *p == '\n' || *p == '\r') {
            needs_quote = 1;
            break;
        }
    }

    if (!needs_quote) {
        fputs(s, fp);
        return;
    }

    fputc('"', fp);
    for (const char *p = s; *p; p++) {
        if (*p == '"') fputc('"', fp);
        fputc(*p, fp);
    }
    fputc('"', fp);
}

/* 시간표 표를 어디까지 출력할지 정하기 위해 가장 늦게 끝나는 교시를 찾습니다. */
static int last_used_period(const Schedule *s) {
    int last = 1;
    for (int i = 0; i < s->count; i++) {
        int end = course_list[s->indices[i]].end_period;
        if (end > last) last = end;
    }
    return last;
}

/* 요일을 열로, 교시를 행으로 하는 시간표 격자를 CSV에 씁니다. */
static void write_grid(FILE *fp, const Schedule *s) {

    fputs("교시", fp);
    for (int d = 0; d < DAYS_IN_WEEK; d++) {
        fputc(',', fp);
        fputs(DAY_NAMES[d], fp);
    }
    fputc('\n', fp);

    int last = last_used_period(s);
    for (int p = 1; p <= last; p++) {
        fprintf(fp, "%d교시", p);
        for (int d = 0; d < DAYS_IN_WEEK; d++) {
            fputc(',', fp);
            int idx = course_at(s, d, p);
            if (idx >= 0) {
                write_csv_field(fp, course_list[idx].name);
            }
        }
        fputc('\n', fp);
    }
}

/* 총 학점, 평균 평점, 공강 요일, 강의 목록 같은 요약 정보를 CSV에 씁니다. */
static void write_summary(FILE *fp, const Schedule *s) {
    fprintf(fp, "총 학점,%d\n", s->total_credit);
    fprintf(fp, "평균 평점,%.2f\n", s->avg_rating);

    int used[DAYS_IN_WEEK] = {0};
    for (int i = 0; i < s->count; i++) {
        Course *c = &course_list[s->indices[i]];
        for (int d = 0; d < c->day_count; d++) {
            if (c->days[d] >= 0) used[c->days[d]] = 1;
        }
    }

    fputs("공강 요일,", fp);
    int any_free = 0;
    for (int d = 0; d < DAYS_IN_WEEK; d++) {
        if (!used[d]) {
            fputs(DAY_NAMES[d], fp);
            any_free = 1;
        }
    }
    if (!any_free) fputs("없음", fp);
    fputc('\n', fp);

    fprintf(fp, "1교시 수업,%s\n", has_period_1(s) ? "있음" : "없음");

    fputs("\n강의명,교수명,요일,시작교시,종료교시,학점,평점,필수\n", fp);
    for (int i = 0; i < s->count; i++) {
        Course *c = &course_list[s->indices[i]];

        write_csv_field(fp, c->name);
        fputc(',', fp);
        write_csv_field(fp, c->professor);
        fputc(',', fp);
        for (int d = 0; d < c->day_count; d++) {
            fputs(DAY_NAMES[c->days[d]], fp);
        }
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
    FILE *fp = fopen(path, "w");
    if (!fp) {
        printf("결과 파일을 저장할 수 없습니다: %s\n", path);
        return 0;
    }

    /* Excel이 한글 UTF-8 CSV를 잘 인식하도록 BOM을 씁니다. */
    fputc(0xEF, fp);
    fputc(0xBB, fp);
    fputc(0xBF, fp);

    for (int i = 0; i < count; i++) {
        write_csv_field(fp, titles[i]);
        fputc('\n', fp);

        write_grid(fp, &schedules[i]);
        fputc('\n', fp);

        write_summary(fp, &schedules[i]);

        if (i < count - 1) fputs("\n\n", fp);
    }

    fclose(fp);
    return 1;
}
