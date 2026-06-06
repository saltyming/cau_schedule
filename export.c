#include <stdio.h>
#include "export.h"

static const char *DAY_NAMES[] = {"월", "화", "수", "목", "금"};

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

static int last_used_period(const Schedule *s) {
    int last = 1;
    for (int i = 0; i < s->count; i++) {
        int end = course_list[s->indices[i]].end_period;
        if (end > last) last = end;
    }
    return last;
}

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

int save_schedules_csv(const char *path, const char *titles[],
                       const Schedule schedules[], int count) {
    FILE *fp = fopen(path, "w");
    if (!fp) {
        printf("결과 파일을 저장할 수 없습니다: %s\n", path);
        return 0;
    }

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
