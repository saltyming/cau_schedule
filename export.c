#include <stdio.h>
#include "export.h"

// 요일 이름. 표의 머리글과 요약에 함께 씁니다.
static const char *DAY_NAMES[] = {"월", "화", "수", "목", "금"};

// MARK: - 파일 안에서만 쓰는 도우미

/**
 CSV 한 칸을 표준 규칙에 맞게 씁니다.

 칸 안에 쉼표·따옴표·줄바꿈이 있으면 Excel이 칸을 잘못 나눌 수 있으므로,
 그럴 때는 칸 전체를 큰따옴표로 감싸고 내부의 따옴표는 두 번 써서
 표현합니다. (CSV 표준, RFC 4180) 보통의 강의명은 그대로 적힙니다.

 - Parameter fp: 쓸 파일
 - Parameter s: 한 칸에 들어갈 글자
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
        if (*p == '"') fputc('"', fp);   // 따옴표는 두 번 써서 표현합니다.
        fputc(*p, fp);
    }
    fputc('"', fp);
}

/**
 시간표에서 가장 늦은 교시를 찾습니다.

 표를 필요한 높이만큼만 그려, 아래쪽 빈 줄이 길게 남지 않도록 합니다.

 - Parameter s: 대상 시간표
 - Returns: 사용된 가장 늦은 교시 번호. 빈 시간표면 1.
 */
static int last_used_period(const Schedule *s) {
    int last = 1;
    for (int i = 0; i < s->count; i++) {
        int end = course_list[s->indices[i]].end_period;
        if (end > last) last = end;
    }
    return last;
}

/**
 한 시간표를 요일 × 교시 표 형태로 파일에 씁니다.

 한 강의가 여러 교시에 걸치면, 각 교시 칸마다 강의명을 적어 줍니다.
 Excel에서 칸마다 무슨 수업인지 바로 보이도록 하기 위함입니다.

 - Parameter fp: 쓸 파일
 - Parameter s: 표로 그릴 시간표
 */
static void write_grid(FILE *fp, const Schedule *s) {
    // 머리글:  교시,월,화,수,목,금
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

/**
 한 시간표의 요약 정보와 강의 목록을 파일에 씁니다.

 - Parameter fp: 쓸 파일
 - Parameter s: 요약할 시간표
 */
static void write_summary(FILE *fp, const Schedule *s) {
    fprintf(fp, "총 학점,%d\n", s->total_credit);
    fprintf(fp, "평균 평점,%.2f\n", s->avg_rating);

    // 어느 요일에 수업이 있는지 표시한 뒤, 비어 있는 요일을 공강으로 모읍니다.
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

    // 담긴 강의들을 한 줄씩 적습니다. (Excel에서 보기 좋은 작은 표)
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

// MARK: - 공개 함수

/**
 만들어진 시간표들을 하나의 CSV 파일로 저장합니다. (자세한 설명은 export.h)

 - Parameter path: 저장할 파일 경로
 - Parameter titles: 각 시간표의 제목 배열
 - Parameter schedules: 저장할 시간표 배열
 - Parameter count: 저장할 시간표 개수
 - Returns: 저장에 성공하면 1, 파일을 열 수 없으면 0.
 */
int save_schedules_csv(const char *path, const char *titles[],
                       const Schedule schedules[], int count) {
    FILE *fp = fopen(path, "w");
    if (!fp) {
        printf("결과 파일을 저장할 수 없습니다: %s\n", path);
        return 0;
    }

    // UTF-8 표식(BOM) 3바이트. Excel이 파일을 UTF-8로 인식해 한글이 안 깨집니다.
    fputc(0xEF, fp);
    fputc(0xBB, fp);
    fputc(0xBF, fp);

    for (int i = 0; i < count; i++) {
        write_csv_field(fp, titles[i]);   // 구역 제목
        fputc('\n', fp);

        write_grid(fp, &schedules[i]);    // 요일 × 교시 표
        fputc('\n', fp);

        write_summary(fp, &schedules[i]); // 요약 + 강의 목록

        // 다음 시간표와 눈으로 구분되도록 빈 줄 두 개를 넣습니다.
        if (i < count - 1) fputs("\n\n", fp);
    }

    fclose(fp);
    return 1;
}
