#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "course.h"

/*
 * course.c
 * --------
 * 강의 정보를 입력받고, CSV 파일에서 읽고, 강의 목록을 화면에 출력하는 기능을
 * 구현합니다. 문자열 처리와 포인터 이동이 많으므로 각 함수 주석에서 흐름을
 * 자세히 설명합니다.
 */

/*
 * 프로그램 전체에서 공유하는 강의 저장소입니다.
 * course_list는 Course 구조체 100개를 담는 배열이고, course_count는 그중 실제로
 * 몇 칸을 사용 중인지 나타냅니다.
 */
Course course_list[MAX_COURSES];
int    course_count = 0;

/*
 * 정수를 안전하게 입력받습니다.
 * fgets로 한 줄 전체를 문자열로 받은 뒤 sscanf로 정수 하나를 꺼냅니다.
 * 사용자가 잘못 입력하면 for (;;) 무한 반복으로 다시 입력받습니다.
 */
int read_int(const char *prompt, int min_value, int max_value) {
    char line[128];
    for (;;) {
        printf("%s", prompt);
        if (fgets(line, sizeof(line), stdin) == NULL) {
            return min_value;
        }
        int value;
        if (sscanf(line, "%d", &value) == 1 &&
            value >= min_value && value <= max_value) {
            return value;
        }
        printf("  ※ %d ~ %d 사이의 숫자로 입력해 주세요.\n", min_value, max_value);
    }
}

/* read_int와 같은 방식으로 실수(float)를 입력받습니다. */
float read_float(const char *prompt, float min_value, float max_value) {
    char line[128];
    for (;;) {
        printf("%s", prompt);
        if (fgets(line, sizeof(line), stdin) == NULL) {
            return min_value;
        }
        float value;
        if (sscanf(line, "%f", &value) == 1 &&
            value >= min_value && value <= max_value) {
            return value;
        }
        printf("  ※ %.1f ~ %.1f 사이의 숫자로 입력해 주세요.\n", min_value, max_value);
    }
}

/*
 * 문자열 한 줄을 입력받습니다.
 * buf는 호출한 쪽에서 준비한 char 배열의 주소이고, buf_size는 그 배열의 크기입니다.
 */
void read_line(const char *prompt, char *buf, int buf_size) {
    printf("%s", prompt);
    if (fgets(buf, buf_size, stdin) == NULL) {
        buf[0] = '\0';
        return;
    }
    buf[strcspn(buf, "\r\n")] = '\0';
}

/* 요일 번호를 화면에 출력할 한글 문자열로 바꿉니다. */
const char *day_to_str(int d) {
    switch (d) {
        case MON: return "월";
        case TUE: return "화";
        case WED: return "수";
        case THU: return "목";
        case FRI: return "금";
        default:  return "?";
    }
}

/* 문자열 안에서 요일 글자를 찾아 첫 번째로 발견된 요일 번호를 반환합니다. */
int parse_day(const char *s) {
    if (strstr(s, "월")) return MON;
    if (strstr(s, "화")) return TUE;
    if (strstr(s, "수")) return WED;
    if (strstr(s, "목")) return THU;
    if (strstr(s, "금")) return FRI;
    return -1;
}

/*
 * 문자열 앞뒤의 공백 문자를 제거합니다.
 * char *s는 문자열의 첫 글자를 가리키는 포인터입니다. s++를 하면 다음 글자를
 * 가리키게 되므로, 앞쪽 공백을 건너뛸 수 있습니다.
 */
static char *trim(char *s) {
    while (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n') s++;

    char *end = s + strlen(s);
    while (end > s) {
        char c = end[-1];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') end--;
        else break;
    }
    *end = '\0';
    return s;
}

/*
 * CSV 한 줄이 UTF-8 규칙을 따르는지 간단히 검사합니다.
 * 한글은 여러 바이트로 저장되므로, 각 바이트 패턴이 UTF-8 형식에 맞는지 확인합니다.
 */
static int is_valid_utf8(const char *str) {
    const unsigned char *s = (const unsigned char *)str;
    while (*s) {
        if (s[0] < 0x80) {
            s += 1;
        } else if ((s[0] & 0xE0) == 0xC0) {
            if ((s[1] & 0xC0) != 0x80) return 0;
            s += 2;
        } else if ((s[0] & 0xF0) == 0xE0) {
            if ((s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80) return 0;
            s += 3;
        } else if ((s[0] & 0xF8) == 0xF0) {
            if ((s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80 ||
                (s[3] & 0xC0) != 0x80) return 0;
            s += 4;
        } else {
            return 0;
        }
    }
    return 1;
}

/*
 * "월수금" 같은 문자열을 Course의 days 배열에 요일 번호로 저장합니다.
 * Course *c를 받기 때문에 이 함수 안에서 c->days를 바꾸면 원래 Course가 바뀝니다.
 */
static void parse_days_into(Course *c, const char *day_text) {
    static const char *day_names[] = {"월", "화", "수", "목", "금"};
    c->day_count = 0;
    for (int d = 0; d < DAYS_IN_WEEK && c->day_count < MAX_DAYS; d++) {
        if (strstr(day_text, day_names[d])) {
            c->days[c->day_count++] = d;
        }
    }
}

/*
 * 유니코드 문자 하나가 콘솔에서 차지하는 칸 수를 반환합니다.
 * 영어는 보통 1칸, 한글은 보통 2칸을 차지하므로 표 정렬에 이 계산이 필요합니다.
 */
static int codepoint_display_width(unsigned int cp) {
    if (cp == 0 || cp < 32 || (cp >= 0x7F && cp < 0xA0)) return 0;

    if ((cp >= 0x1100 && cp <= 0x115F) ||
        (cp >= 0x2329 && cp <= 0x232A) ||
        (cp >= 0x2E80 && cp <= 0xA4CF && cp != 0x303F) ||
        (cp >= 0xAC00 && cp <= 0xD7A3) ||
        (cp >= 0xF900 && cp <= 0xFAFF) ||
        (cp >= 0xFE10 && cp <= 0xFE19) ||
        (cp >= 0xFE30 && cp <= 0xFE6F) ||
        (cp >= 0xFF00 && cp <= 0xFF60) ||
        (cp >= 0xFFE0 && cp <= 0xFFE6)) {
        return 2;
    }

    return 1;
}

/*
 * UTF-8 문자열에서 문자 하나를 읽고, 포인터를 다음 문자 위치로 이동시킵니다.
 * const char **s는 "문자열 포인터의 주소"입니다. 함수 안에서 *s 값을 바꾸면
 * 호출한 쪽의 문자열 포인터도 다음 위치를 가리키게 됩니다.
 */
static unsigned int next_utf8_codepoint(const char **s) {
    const unsigned char *p = (const unsigned char *)*s;

    if (p[0] < 0x80) {
        *s += 1;
        return p[0];
    }

    if ((p[0] & 0xE0) == 0xC0 &&
        p[1] != '\0' &&
        (p[1] & 0xC0) == 0x80) {
        *s += 2;
        return ((unsigned int)(p[0] & 0x1F) << 6) |
               (unsigned int)(p[1] & 0x3F);
    }

    if ((p[0] & 0xF0) == 0xE0 &&
        p[1] != '\0' &&
        p[2] != '\0' &&
        (p[1] & 0xC0) == 0x80 &&
        (p[2] & 0xC0) == 0x80) {
        *s += 3;
        return ((unsigned int)(p[0] & 0x0F) << 12) |
               ((unsigned int)(p[1] & 0x3F) << 6) |
               (unsigned int)(p[2] & 0x3F);
    }

    if ((p[0] & 0xF8) == 0xF0 &&
        p[1] != '\0' &&
        p[2] != '\0' &&
        p[3] != '\0' &&
        (p[1] & 0xC0) == 0x80 &&
        (p[2] & 0xC0) == 0x80 &&
        (p[3] & 0xC0) == 0x80) {
        *s += 4;
        return ((unsigned int)(p[0] & 0x07) << 18) |
               ((unsigned int)(p[1] & 0x3F) << 12) |
               ((unsigned int)(p[2] & 0x3F) << 6) |
               (unsigned int)(p[3] & 0x3F);
    }

    *s += 1;
    return p[0];
}

/* 문자열 전체가 콘솔에서 차지하는 폭을 계산합니다. */
static int display_width(const char *s) {
    int width = 0;
    while (*s) {
        unsigned int cp = next_utf8_codepoint(&s);
        width += codepoint_display_width(cp);
    }
    return width;
}

/* 두 정수 중 큰 값을 반환합니다. */
static int max_int(int a, int b) {
    return (a > b) ? a : b;
}

/* 표의 빈 칸을 맞추기 위해 공백을 count개 출력합니다. */
static void print_padding(int count) {
    for (int i = 0; i < count; i++) putchar(' ');
}

/* 표 한 칸을 지정한 폭에 맞춰 왼쪽 또는 오른쪽 정렬로 출력합니다. */
static void print_cell(const char *text, int width, int right_align) {
    int padding = width - display_width(text);
    if (padding < 0) padding = 0;

    if (right_align) print_padding(padding);
    fputs(text, stdout);
    if (!right_align) print_padding(padding);
}

/* 표 제목 아래에 들어가는 구분선(-)을 출력합니다. */
static void print_rule_cell(int width) {
    for (int i = 0; i < width; i++) putchar('-');
}

/* Course의 요일 배열을 "월수금" 같은 출력용 문자열로 만듭니다. */
static void print_course_days(const Course *c, char *buf, size_t buf_size) {
    if (buf_size == 0) return;

    buf[0] = '\0';
    for (int d = 0; d < c->day_count; d++) {
        const char *day = day_to_str(c->days[d]);
        size_t used = strlen(buf);
        size_t day_len = strlen(day);
        if (used + day_len >= buf_size) break;
        strcat(buf, day);
    }
}

/*
 * 사용자가 콘솔에서 직접 강의 정보를 입력하는 함수입니다.
 * 입력된 강의는 전역 배열 course_list의 다음 빈 칸에 저장됩니다.
 */
void input_courses_manual(void) {
    int n = read_int("\n입력할 강의 수: ", 0, MAX_COURSES);

    for (int i = 0; i < n && course_count < MAX_COURSES; i++) {
        Course *c = &course_list[course_count];
        memset(c, 0, sizeof(Course));
        for (int d = 0; d < MAX_DAYS; d++) c->days[d] = -1;

        printf("\n─── 강의 %d ───\n", i + 1);

        read_line("강의명: ", c->name, MAX_NAME_LEN);
        read_line("교수명: ", c->professor, MAX_PROF_LEN);

        char day_buf[32];
        read_line("요일 (예: 월수 / 월수금 / 화목): ", day_buf, sizeof(day_buf));
        /* 입력받은 요일 문자열을 숫자 배열로 바꿔 Course 안에 저장합니다. */
        parse_days_into(c, day_buf);
        if (c->day_count == 0) {
            printf("  ※ 요일을 알아보지 못해 월요일로 설정합니다.\n");
            c->days[0] = MON;
            c->day_count = 1;
        }

        c->start_period = read_int("시작 교시 (1~9): ", 1, MAX_PERIOD);
        c->end_period   = read_int("종료 교시 (시작 교시 이상): ", c->start_period, MAX_PERIOD);
        c->credit       = read_int("학점 (1~6): ", 1, 6);
        c->rating       = read_float("강의 평점 (0.0~5.0): ", 0.0f, 5.0f);
        c->is_required  = read_int("필수 여부 (1=필수, 0=선택): ", 0, 1);

        course_count++;
    }
    printf("\n[완료] 강의 %d개 입력 완료.\n", n);
}

/*
 * CSV 파일에서 강의 목록을 읽어 course_list에 추가합니다.
 * filename은 파일 이름 문자열의 주소입니다. 파일을 열 수 없으면 -1을 반환합니다.
 */
int load_courses_from_csv(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("파일을 열 수 없습니다: %s\n", filename);
        return -1;
    }

    /* UTF-8 BOM이 있으면 읽어서 건너뛰고, 없으면 rewind로 파일 처음으로 돌아갑니다. */
    unsigned char bom[3] = {0};
    size_t got = fread(bom, 1, 3, fp);
    if (!(got == 3 && bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF)) {
        rewind(fp);
    }

    char line[256];
    int loaded = 0;
    int line_no = 1;
    int warned_encoding = 0;

    /* 첫 줄은 CSV 제목 행이므로 읽기만 하고 실제 강의로 저장하지 않습니다. */
    if (fgets(line, sizeof(line), fp) == NULL) {
        fclose(fp);
        printf("[완료] CSV에서 강의 0개 로드 완료.\n");
        return 0;
    }

    while (fgets(line, sizeof(line), fp) && course_count < MAX_COURSES) {
        line_no++;

        /* 빈 줄인지 확인하기 위해 p 포인터를 줄 앞쪽 공백 뒤로 이동합니다. */
        const char *p = line;
        while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
        if (*p == '\0') continue;

        if (!warned_encoding && !is_valid_utf8(line)) {
            printf("\n[경고] 이 CSV 파일은 UTF-8이 아닌 것 같습니다. 한글이 깨질 수 있어요.\n");
            printf("   Excel에서 '다른 이름으로 저장 → CSV UTF-8'로 다시 저장해 주세요.\n\n");
            warned_encoding = 1;
        }

        /*
         * strtok은 line 문자열을 쉼표 기준으로 잘라 각 칸의 시작 주소를 반환합니다.
         * 두 번째 호출부터 NULL을 넣는 것은 "직전 문자열에서 계속 자르라"는 뜻입니다.
         */
        char *name_tok  = strtok(line, ",");
        char *prof_tok  = strtok(NULL, ",");
        char *day_tok   = strtok(NULL, ",");
        char *start_tok = strtok(NULL, ",");
        char *end_tok   = strtok(NULL, ",");
        char *cred_tok  = strtok(NULL, ",");
        char *rate_tok  = strtok(NULL, ",");
        char *req_tok   = strtok(NULL, ",\r\n");

        if (!name_tok || !prof_tok || !day_tok || !start_tok ||
            !end_tok || !cred_tok || !rate_tok || !req_tok) {
            printf("  ※ %d번째 줄: 칸 수가 부족해 건너뜁니다.\n", line_no);
            continue;
        }

        /* 각 칸의 앞뒤 공백을 제거하고, 숫자 칸은 atoi/atof로 숫자 타입으로 바꿉니다. */
        char *name     = trim(name_tok);
        char *prof     = trim(prof_tok);
        char *days     = trim(day_tok);
        int   start    = atoi(trim(start_tok));
        int   end      = atoi(trim(end_tok));
        int   credit   = atoi(trim(cred_tok));
        float rating   = (float)atof(trim(rate_tok));
        int   required = atoi(trim(req_tok));

        if (name[0] == '\0') {
            printf("  ※ %d번째 줄: 강의명이 비어 건너뜁니다.\n", line_no);
            continue;
        }
        if (start < 1 || start > MAX_PERIOD || end < 1 || end > MAX_PERIOD || start > end) {
            printf("  ※ %d번째 줄(%s): 교시 값이 올바르지 않아 건너뜁니다.\n", line_no, name);
            continue;
        }
        if (credit < 1 || credit > 30) {
            printf("  ※ %d번째 줄(%s): 학점 값이 올바르지 않아 건너뜁니다.\n", line_no, name);
            continue;
        }
        if (rating < 0.0f || rating > 5.0f) {
            printf("  ※ %d번째 줄(%s): 평점 값이 올바르지 않아 건너뜁니다.\n", line_no, name);
            continue;
        }

        required = (required == 1) ? 1 : 0;

        /* 검증을 통과한 CSV 한 줄을 Course 구조체에 복사합니다. */
        Course *c = &course_list[course_count];
        memset(c, 0, sizeof(Course));
        for (int d = 0; d < MAX_DAYS; d++) c->days[d] = -1;

        strncpy(c->name, name, MAX_NAME_LEN - 1);
        strncpy(c->professor, prof, MAX_PROF_LEN - 1);
        c->start_period = start;
        c->end_period   = end;
        c->credit       = credit;
        c->rating       = rating;
        c->is_required  = required;

        parse_days_into(c, days);
        if (c->day_count == 0) {
            printf("  ※ %d번째 줄(%s): 요일을 알아보지 못해 월요일로 설정합니다.\n",
                   line_no, name);
            c->days[0] = MON;
            c->day_count = 1;
        }

        course_count++;
        loaded++;
    }

    fclose(fp);
    printf("[완료] CSV에서 강의 %d개 로드 완료.\n", loaded);
    return loaded;
}

/*
 * 현재 저장된 강의 목록을 표 형태로 출력합니다.
 * 먼저 각 열에 필요한 폭을 계산한 뒤, 제목/구분선/데이터 행을 차례로 출력합니다.
 */
void print_course_list(void) {
    int no_width = 4;
    int name_width = 20;
    int professor_width = 12;
    int day_width = 8;
    int number_width = 5;
    int required_width = 6;

    /* 1단계: 모든 강의를 훑으며 각 열의 최대 출력 폭을 구합니다. */
    for (int i = 0; i < course_count; i++) {
        Course *c = &course_list[i];

        char day_str[20];
        print_course_days(c, day_str, sizeof(day_str));

        name_width = max_int(name_width, display_width(c->name));
        professor_width = max_int(professor_width, display_width(c->professor));
        day_width = max_int(day_width, display_width(day_str));
        required_width = max_int(required_width,
                                 display_width(c->is_required ? "필수" : "선택"));
    }

    /* 2단계: 표 제목과 제목 아래 구분선을 출력합니다. */
    printf("\n");
    print_cell("No", no_width, 0);           putchar(' ');
    print_cell("강의명", name_width, 0);     putchar(' ');
    print_cell("교수명", professor_width, 0); putchar(' ');
    print_cell("요일", day_width, 0);        putchar(' ');
    print_cell("시작", number_width, 1);     putchar(' ');
    print_cell("종료", number_width, 1);     putchar(' ');
    print_cell("학점", number_width, 1);     putchar(' ');
    print_cell("평점", number_width, 1);     putchar(' ');
    print_cell("필수", required_width, 1);   putchar('\n');

    print_rule_cell(no_width);        putchar(' ');
    print_rule_cell(name_width);      putchar(' ');
    print_rule_cell(professor_width); putchar(' ');
    print_rule_cell(day_width);       putchar(' ');
    print_rule_cell(number_width);    putchar(' ');
    print_rule_cell(number_width);    putchar(' ');
    print_rule_cell(number_width);    putchar(' ');
    print_rule_cell(number_width);    putchar(' ');
    print_rule_cell(required_width);  putchar('\n');

    /* 3단계: 각 강의를 한 줄씩 출력합니다. 숫자는 문자열로 바꾼 뒤 표 칸에 맞춥니다. */
    for (int i = 0; i < course_count; i++) {
        Course *c = &course_list[i];

        char day_str[20];
        char no_text[16];
        char start_text[16];
        char end_text[16];
        char credit_text[16];
        char rating_text[16];

        print_course_days(c, day_str, sizeof(day_str));
        snprintf(no_text, sizeof(no_text), "%d", i + 1);
        snprintf(start_text, sizeof(start_text), "%d", c->start_period);
        snprintf(end_text, sizeof(end_text), "%d", c->end_period);
        snprintf(credit_text, sizeof(credit_text), "%d", c->credit);
        snprintf(rating_text, sizeof(rating_text), "%.1f", c->rating);

        print_cell(no_text, no_width, 0);                    putchar(' ');
        print_cell(c->name, name_width, 0);                  putchar(' ');
        print_cell(c->professor, professor_width, 0);        putchar(' ');
        print_cell(day_str, day_width, 0);                   putchar(' ');
        print_cell(start_text, number_width, 1);             putchar(' ');
        print_cell(end_text, number_width, 1);               putchar(' ');
        print_cell(credit_text, number_width, 1);            putchar(' ');
        print_cell(rating_text, number_width, 1);            putchar(' ');
        print_cell(c->is_required ? "필수" : "선택", required_width, 1);
        putchar('\n');
    }
}
