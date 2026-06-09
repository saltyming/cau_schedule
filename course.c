// 화면 출력·입력(printf, fgets 등)을 쓰기 위한 표준 입출력 라이브러리
#include <stdio.h>
// 문자열을 숫자로 바꾸는 atoi/atof 등을 쓰기 위한 표준 라이브러리
#include <stdlib.h>
// 문자열 다루기(strstr, strtok, strncpy 등)를 위한 라이브러리
#include <string.h>
// 이 파일에서 쓰는 Course 구조체와 상수, 함수 약속이 들어 있는 헤더
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
// 모든 강의를 담아 두는 실제 배열(헤더의 extern 선언이 가리키는 진짜 알맹이)
Course course_list[MAX_COURSES];
// 지금까지 채워 넣은 강의 개수(처음에는 0개)
int    course_count = 0;

/*
 * 정수를 안전하게 입력받습니다.
 * fgets로 한 줄 전체를 문자열로 받은 뒤 sscanf로 정수 하나를 꺼냅니다.
 * 사용자가 잘못 입력하면 for (;;) 무한 반복으로 다시 입력받습니다.
 */
int read_int(const char *prompt, int min_value, int max_value) {
    // 사용자가 입력한 한 줄을 통째로 담아 둘 임시 문자열 상자(최대 127글자)
    char line[128];
    // 올바른 값이 들어올 때까지 끝없이 반복한다(중간에 return 으로 빠져나간다)
    for (;;) {
        // 무엇을 입력해야 하는지 안내 문구(prompt)를 화면에 보여 준다
        printf("%s", prompt);
        // 키보드(stdin)에서 한 줄을 읽는다. 더 읽을 게 없으면 NULL 이 돌아온다
        if (fgets(line, sizeof(line), stdin) == NULL) {
            // 입력이 끊겼으면 최소값을 돌려주고 함수를 끝낸다
            return min_value;
        }
        // 읽은 글자에서 뽑아낸 정수를 담을 변수
        int value;
        // 읽은 줄에서 정수 하나를 뽑아낸다. 성공하면 sscanf 가 1을 돌려주고,
        if (sscanf(line, "%d", &value) == 1 &&
            // 그 값이 허용 범위(min~max) 안에 있는지도 함께 확인한다
            value >= min_value && value <= max_value) {
            // 두 조건을 모두 만족하면 그 값을 돌려주고 끝낸다
            return value;
        }
        // 잘못된 입력이면 안내를 보여 주고, 반복문이 처음으로 돌아가 다시 받는다
        printf("  ※ %d ~ %d 사이의 숫자로 입력해 주세요.\n", min_value, max_value);
    }
}

/* read_int와 같은 방식으로 실수(float)를 입력받습니다. */
float read_float(const char *prompt, float min_value, float max_value) {
    // 입력한 한 줄을 담아 둘 임시 문자열 상자
    char line[128];
    // 올바른 실수가 들어올 때까지 반복한다
    for (;;) {
        // 안내 문구를 보여 준다
        printf("%s", prompt);
        // 키보드에서 한 줄을 읽는다. 끊기면 NULL
        if (fgets(line, sizeof(line), stdin) == NULL) {
            // 입력이 끊겼으면 최소값을 돌려준다
            return min_value;
        }
        // 뽑아낸 실수를 담을 변수
        float value;
        // 읽은 줄에서 실수 하나를 뽑아내고(성공 시 1),
        if (sscanf(line, "%f", &value) == 1 &&
            // 허용 범위 안에 있는지 확인한다
            value >= min_value && value <= max_value) {
            // 조건을 만족하면 그 값을 돌려준다
            return value;
        }
        // 잘못된 입력이면 안내 후 다시 입력받는다
        printf("  ※ %.1f ~ %.1f 사이의 숫자로 입력해 주세요.\n", min_value, max_value);
    }
}

/*
 * 문자열 한 줄을 입력받습니다.
 * buf는 호출한 쪽에서 준비한 char 배열의 주소이고, buf_size는 그 배열의 크기입니다.
 */
void read_line(const char *prompt, char *buf, int buf_size) {
    // 안내 문구를 보여 준다
    printf("%s", prompt);
    // 키보드에서 한 줄을 읽어 buf 에 담는다. 끊기면 NULL
    if (fgets(buf, buf_size, stdin) == NULL) {
        // 읽지 못했으면 빈 문자열로 만든다('\0'은 문자열의 끝 표시)
        buf[0] = '\0';
        // 함수를 끝낸다
        return;
    }
    // strcspn 은 첫 줄바꿈(\r 또는 \n) 위치를 찾아 준다. 그 자리에 끝 표시를 넣어
    // 입력에 딸려 온 줄바꿈 문자를 잘라낸다
    buf[strcspn(buf, "\r\n")] = '\0';
}

/* 요일 번호를 화면에 출력할 한글 문자열로 바꿉니다. */
const char *day_to_str(int d) {
    // 요일 번호 d 값에 따라 알맞은 글자를 골라 돌려준다
    switch (d) {
        // 0이면 "월"
        case MON: return "월";
        // 1이면 "화"
        case TUE: return "화";
        // 2이면 "수"
        case WED: return "수";
        // 3이면 "목"
        case THU: return "목";
        // 4이면 "금"
        case FRI: return "금";
        // 그 밖의 알 수 없는 값이면 물음표
        default:  return "?";
    }
}

/* 문자열 안에서 요일 글자를 찾아 첫 번째로 발견된 요일 번호를 반환합니다. */
int parse_day(const char *s) {
    // strstr 은 문자열 s 안에 "월"이 들어 있으면 그 위치를, 없으면 NULL 을 준다
    if (strstr(s, "월")) return MON;
    // "화"가 들어 있으면 화요일 번호
    if (strstr(s, "화")) return TUE;
    // "수"가 들어 있으면 수요일 번호
    if (strstr(s, "수")) return WED;
    // "목"이 들어 있으면 목요일 번호
    if (strstr(s, "목")) return THU;
    // "금"이 들어 있으면 금요일 번호
    if (strstr(s, "금")) return FRI;
    // 어떤 요일 글자도 못 찾으면 -1(없음)
    return -1;
}

/*
 * 문자열 앞뒤의 공백 문자를 제거합니다.
 * char *s는 문자열의 첫 글자를 가리키는 포인터입니다. s++를 하면 다음 글자를
 * 가리키게 되므로, 앞쪽 공백을 건너뛸 수 있습니다.
 */
static char *trim(char *s) {
    // 맨 앞 글자가 공백/탭/줄바꿈인 동안 포인터를 한 칸씩 뒤로 옮겨 건너뛴다
    while (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n') s++;

    // 문자열의 끝(널 문자 자리)을 가리키는 포인터를 만든다. strlen 은 글자 수
    char *end = s + strlen(s);
    // 끝에서부터 시작 위치 s 까지 거꾸로 살펴본다
    while (end > s) {
        // end 바로 앞 글자(현재의 마지막 글자)를 꺼낸다. end[-1]은 한 칸 앞 칸
        char c = end[-1];
        // 그 글자가 공백류이면 end 를 한 칸 앞으로 당겨 끝을 줄인다
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') end--;
        // 공백이 아니면 더 이상 지울 게 없으니 멈춘다
        else break;
    }
    // 줄어든 끝 위치에 끝 표시를 넣어 뒤쪽 공백을 실제로 잘라낸다
    *end = '\0';
    // 앞 공백을 건너뛴 새 시작 위치를 돌려준다
    return s;
}

/*
 * CSV 한 줄이 UTF-8 규칙을 따르는지 간단히 검사합니다.
 * 한글은 여러 바이트로 저장되므로, 각 바이트 패턴이 UTF-8 형식에 맞는지 확인합니다.
 */
static int is_valid_utf8(const char *str) {
    // 각 글자를 0~255 숫자로 보기 위해 부호 없는(unsigned) 바이트 포인터로 바꾼다
    const unsigned char *s = (const unsigned char *)str;
    // 문자열 끝(0)을 만날 때까지 한 바이트씩 검사한다
    while (*s) {
        // 첫 바이트가 128(0x80)보다 작으면 평범한 영어·숫자 한 글자(1바이트)
        if (s[0] < 0x80) {
            // 1바이트만 차지하므로 한 칸 전진
            s += 1;
        // 위 3비트가 110이면(0xE0=11100000 으로 가린 값이 0xC0=11000000) 2바이트 글자 시작
        } else if ((s[0] & 0xE0) == 0xC0) {
            // 뒤따르는 바이트는 위 2비트가 10(0x80)이어야 한다. 아니면 잘못된 형식(0)
            if ((s[1] & 0xC0) != 0x80) return 0;
            // 2바이트를 차지하므로 두 칸 전진
            s += 2;
        // 위 4비트가 1110이면(0xF0 으로 가린 값이 0xE0) 3바이트 글자(한글 대부분)
        } else if ((s[0] & 0xF0) == 0xE0) {
            // 이어지는 두 바이트가 모두 10으로 시작해야 한다
            if ((s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80) return 0;
            // 3바이트 전진
            s += 3;
        // 위 5비트가 11110이면(0xF8 으로 가린 값이 0xF0) 4바이트 글자(이모지 등)
        } else if ((s[0] & 0xF8) == 0xF0) {
            // 이어지는 세 바이트가 모두 10으로 시작해야 한다
            if ((s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80 ||
                (s[3] & 0xC0) != 0x80) return 0;
            // 4바이트 전진
            s += 4;
        // 어느 형식에도 맞지 않으면 깨진 데이터
        } else {
            // 잘못된 UTF-8 이므로 0(아님)을 돌려준다
            return 0;
        }
    }
    // 끝까지 문제가 없으면 올바른 UTF-8(1)
    return 1;
}

/*
 * "월수금" 같은 문자열을 Course의 days 배열에 요일 번호로 저장합니다.
 * Course *c를 받기 때문에 이 함수 안에서 c->days를 바꾸면 원래 Course가 바뀝니다.
 */
static void parse_days_into(Course *c, const char *day_text) {
    // 비교에 쓸 요일 글자표. static 이라 함수가 다시 불려도 한 번만 만들어 둔다
    static const char *day_names[] = {"월", "화", "수", "목", "금"};
    // 찾은 요일 개수를 0으로 초기화한다
    c->day_count = 0;
    // 월~금을 순서대로 확인하되, days 배열의 칸 수(MAX_DAYS)를 넘지 않게 한다
    for (int d = 0; d < DAYS_IN_WEEK && c->day_count < MAX_DAYS; d++) {
        // 입력 문자열 안에 그 요일 글자가 들어 있으면
        if (strstr(day_text, day_names[d])) {
            // 요일 번호 d 를 days 배열에 넣고, 개수를 1 늘린다
            c->days[c->day_count++] = d;
        }
    }
}

/*
 * 유니코드 문자 하나가 콘솔에서 차지하는 칸 수를 반환합니다.
 * 영어는 보통 1칸, 한글은 보통 2칸을 차지하므로 표 정렬에 이 계산이 필요합니다.
 */
static int codepoint_display_width(unsigned int cp) {
    // 눈에 보이지 않는 제어문자(글자 번호 0, 32 미만, 일부 특수 구간)는 폭 0
    if (cp == 0 || cp < 32 || (cp >= 0x7F && cp < 0xA0)) return 0;

    // 아래 범위들은 모두 "넓은 글자"(2칸)에 해당한다
    if ((cp >= 0x1100 && cp <= 0x115F) ||      // 한글 자모(첫소리) 영역
        (cp >= 0x2329 && cp <= 0x232A) ||      // 각진 괄호 기호
        (cp >= 0x2E80 && cp <= 0xA4CF && cp != 0x303F) ||  // 한자·부수 등 동아시아 글자
        (cp >= 0xAC00 && cp <= 0xD7A3) ||      // 완성형 한글 "가"~"힣"
        (cp >= 0xF900 && cp <= 0xFAFF) ||      // 호환용 한자
        (cp >= 0xFE10 && cp <= 0xFE19) ||      // 세로쓰기용 기호
        (cp >= 0xFE30 && cp <= 0xFE6F) ||      // 한자 호환 기호
        (cp >= 0xFF00 && cp <= 0xFF60) ||      // 전각(넓은) 영문·기호
        (cp >= 0xFFE0 && cp <= 0xFFE6)) {      // 전각 통화 기호 등
        // 위 범위에 들면 콘솔에서 2칸을 차지한다
        return 2;
    }

    // 그 밖의 평범한 글자는 1칸
    return 1;
}

/*
 * UTF-8 문자열에서 문자 하나를 읽고, 포인터를 다음 문자 위치로 이동시킵니다.
 * const char **s는 "문자열 포인터의 주소"입니다. 함수 안에서 *s 값을 바꾸면
 * 호출한 쪽의 문자열 포인터도 다음 위치를 가리키게 됩니다.
 */
static unsigned int next_utf8_codepoint(const char **s) {
    // 지금 읽을 위치의 바이트들을 0~255 숫자로 보기 위한 포인터
    const unsigned char *p = (const unsigned char *)*s;

    // 첫 바이트가 128 미만이면 1바이트짜리 평범한 글자
    if (p[0] < 0x80) {
        // 바깥쪽 포인터를 1칸 전진시켜 다음 글자를 가리키게 한다
        *s += 1;
        // 그 바이트 값이 곧 글자 번호이므로 그대로 돌려준다
        return p[0];
    }

    // 위 3비트가 110이면 2바이트 글자이고,
    if ((p[0] & 0xE0) == 0xC0 &&
        // 두 번째 바이트가 실제로 존재하고,
        p[1] != '\0' &&
        // 그 바이트의 위 2비트가 10(이어지는 바이트 표시)이면
        (p[1] & 0xC0) == 0x80) {
        // 2칸 전진
        *s += 2;
        // 첫 바이트의 아래 5비트(0x1F)를 왼쪽으로 6칸 밀고,
        return ((unsigned int)(p[0] & 0x1F) << 6) |
               // 둘째 바이트의 아래 6비트(0x3F)와 합쳐 글자 번호를 만든다
               (unsigned int)(p[1] & 0x3F);
    }

    // 위 4비트가 1110이면 3바이트 글자이고,
    if ((p[0] & 0xF0) == 0xE0 &&
        // 둘째 바이트가 존재하고,
        p[1] != '\0' &&
        // 셋째 바이트도 존재하고,
        p[2] != '\0' &&
        // 둘째 바이트가 10으로 시작하고,
        (p[1] & 0xC0) == 0x80 &&
        // 셋째 바이트도 10으로 시작하면
        (p[2] & 0xC0) == 0x80) {
        // 3칸 전진
        *s += 3;
        // 첫 바이트의 아래 4비트(0x0F)를 12칸 밀고,
        return ((unsigned int)(p[0] & 0x0F) << 12) |
               // 둘째 바이트의 아래 6비트를 6칸 밀고,
               ((unsigned int)(p[1] & 0x3F) << 6) |
               // 셋째 바이트의 아래 6비트와 모두 합친다
               (unsigned int)(p[2] & 0x3F);
    }

    // 위 5비트가 11110이면 4바이트 글자이고,
    if ((p[0] & 0xF8) == 0xF0 &&
        // 둘째·셋째·넷째 바이트가 모두 존재하고,
        p[1] != '\0' &&
        p[2] != '\0' &&
        p[3] != '\0' &&
        // 그 세 바이트가 모두 10으로 시작하면
        (p[1] & 0xC0) == 0x80 &&
        (p[2] & 0xC0) == 0x80 &&
        (p[3] & 0xC0) == 0x80) {
        // 4칸 전진
        *s += 4;
        // 첫 바이트의 아래 3비트(0x07)를 18칸 밀고,
        return ((unsigned int)(p[0] & 0x07) << 18) |
               // 둘째 바이트의 아래 6비트를 12칸 밀고,
               ((unsigned int)(p[1] & 0x3F) << 12) |
               // 셋째 바이트의 아래 6비트를 6칸 밀고,
               ((unsigned int)(p[2] & 0x3F) << 6) |
               // 넷째 바이트의 아래 6비트와 모두 합친다
               (unsigned int)(p[3] & 0x3F);
    }

    // 위 어느 형식에도 안 맞는 깨진 바이트면, 안전하게 1칸만 전진하고
    *s += 1;
    // 그 바이트 값을 그대로 돌려준다
    return p[0];
}

/* 문자열 전체가 콘솔에서 차지하는 폭을 계산합니다. */
static int display_width(const char *s) {
    // 폭의 합을 담을 변수(0에서 시작)
    int width = 0;
    // 문자열 끝을 만날 때까지 글자를 하나씩 읽는다
    while (*s) {
        // 글자 하나를 읽고 s 를 다음 글자로 옮긴다. 읽은 글자 번호를 cp 에 담는다
        unsigned int cp = next_utf8_codepoint(&s);
        // 그 글자의 칸 수(1 또는 2)를 폭의 합에 더한다
        width += codepoint_display_width(cp);
    }
    // 글자들의 총 폭을 돌려준다
    return width;
}

/* 두 정수 중 큰 값을 반환합니다. */
static int max_int(int a, int b) {
    // a 가 b 보다 크면 a 를, 아니면 b 를 돌려준다(삼항 연산자)
    return (a > b) ? a : b;
}

/* 표의 빈 칸을 맞추기 위해 공백을 count개 출력합니다. */
static void print_padding(int count) {
    // count 번 반복하며 공백 한 칸씩 출력한다
    for (int i = 0; i < count; i++) putchar(' ');
}

/* 표 한 칸을 지정한 폭에 맞춰 왼쪽 또는 오른쪽 정렬로 출력합니다. */
static void print_cell(const char *text, int width, int right_align) {
    // 목표 폭에서 글자의 실제 폭을 빼서, 채워 넣을 공백 수를 구한다
    int padding = width - display_width(text);
    // 글자가 칸보다 길면 음수가 되므로 0으로 맞춘다
    if (padding < 0) padding = 0;

    // 오른쪽 정렬이면 글자 앞에 먼저 공백을 채운다
    if (right_align) print_padding(padding);
    // 실제 글자를 출력한다
    fputs(text, stdout);
    // 왼쪽 정렬이면 글자 뒤에 공백을 채운다
    if (!right_align) print_padding(padding);
}

/* 표 제목 아래에 들어가는 구분선(-)을 출력합니다. */
static void print_rule_cell(int width) {
    // 칸 폭만큼 빼기 기호('-')를 출력한다
    for (int i = 0; i < width; i++) putchar('-');
}

/* Course의 요일 배열을 "월수금" 같은 출력용 문자열로 만듭니다. */
static void print_course_days(const Course *c, char *buf, size_t buf_size) {
    // 담을 공간이 아예 없으면 그냥 끝낸다
    if (buf_size == 0) return;

    // 결과 문자열을 빈 상태("")로 시작한다
    buf[0] = '\0';
    // 이 강의의 요일 개수만큼 반복한다
    for (int d = 0; d < c->day_count; d++) {
        // 요일 번호를 "월" 같은 글자로 바꾼다
        const char *day = day_to_str(c->days[d]);
        // 지금까지 만든 문자열의 길이
        size_t used = strlen(buf);
        // 이번에 붙일 요일 글자의 길이
        size_t day_len = strlen(day);
        // 붙이면 공간을 넘치는 경우 더 붙이지 않고 멈춘다(넘침 방지)
        if (used + day_len >= buf_size) break;
        // 요일 글자를 결과 문자열 뒤에 이어 붙인다
        strcat(buf, day);
    }
}

/*
 * 사용자가 콘솔에서 직접 강의 정보를 입력하는 함수입니다.
 * 입력된 강의는 전역 배열 course_list의 다음 빈 칸에 저장됩니다.
 */
void input_courses_manual(void) {
    // 몇 개의 강의를 입력할지 먼저 물어본다(0~최대 개수)
    int n = read_int("\n입력할 강의 수: ", 0, MAX_COURSES);

    // n 개를 입력받되, 저장 공간이 가득 차면 멈춘다
    for (int i = 0; i < n && course_count < MAX_COURSES; i++) {
        // 이번에 채울 빈 칸(다음 강의 자리)의 주소를 c 에 담는다
        Course *c = &course_list[course_count];
        // 그 칸을 0으로 깨끗이 비운다(이전 쓰레기 값 제거)
        memset(c, 0, sizeof(Course));
        // 요일 칸들을 -1(없음)로 초기화한다
        for (int d = 0; d < MAX_DAYS; d++) c->days[d] = -1;

        // 지금 몇 번째 강의를 입력 중인지 알려 준다
        printf("\n─── 강의 %d ───\n", i + 1);

        // 강의명을 입력받아 c->name 에 담는다
        read_line("강의명: ", c->name, MAX_NAME_LEN);
        // 교수명을 입력받아 c->professor 에 담는다
        read_line("교수명: ", c->professor, MAX_PROF_LEN);

        // 요일 입력을 받아 둘 임시 문자열
        char day_buf[32];
        // "월수금" 같은 요일 문자열을 입력받는다
        read_line("요일 (예: 월수 / 월수금 / 화목): ", day_buf, sizeof(day_buf));
        // 입력받은 요일 문자열을 숫자 배열로 바꿔 Course 안에 저장한다
        parse_days_into(c, day_buf);
        // 요일을 하나도 알아보지 못했다면
        if (c->day_count == 0) {
            // 안내를 보여 주고
            printf("  ※ 요일을 알아보지 못해 월요일로 설정합니다.\n");
            // 기본값으로 월요일 하나를 넣는다
            c->days[0] = MON;
            c->day_count = 1;
        }

        // 시작 교시(1~9)를 입력받는다
        c->start_period = read_int("시작 교시 (1~9): ", 1, MAX_PERIOD);
        // 종료 교시(시작 교시 이상)를 입력받는다
        c->end_period   = read_int("종료 교시 (시작 교시 이상): ", c->start_period, MAX_PERIOD);
        // 학점(1~6)을 입력받는다
        c->credit       = read_int("학점 (1~6): ", 1, 6);
        // 평점(0.0~5.0)을 입력받는다
        c->rating       = read_float("강의 평점 (0.0~5.0): ", 0.0f, 5.0f);
        // 필수 여부(1 또는 0)를 입력받는다
        c->is_required  = read_int("필수 여부 (1=필수, 0=선택): ", 0, 1);

        // 강의 한 개를 다 채웠으니 전체 개수를 1 늘린다
        course_count++;
    }
    // 입력이 끝났음을 알려 준다
    printf("\n[완료] 강의 %d개 입력 완료.\n", n);
}

/*
 * CSV 파일에서 강의 목록을 읽어 course_list에 추가합니다.
 * filename은 파일 이름 문자열의 주소입니다. 파일을 열 수 없으면 -1을 반환합니다.
 */
int load_courses_from_csv(const char *filename) {
    // 파일을 읽기 모드("r")로 연다. fp 는 그 파일을 다루는 손잡이
    FILE *fp = fopen(filename, "r");
    // 파일을 열지 못했으면(없거나 권한 문제)
    if (!fp) {
        // 안내를 보여 주고
        printf("파일을 열 수 없습니다: %s\n", filename);
        // 실패 표시(-1)를 돌려준다
        return -1;
    }

    // 파일 맨 앞 3바이트(BOM)를 담을 작은 상자를 0으로 준비한다
    unsigned char bom[3] = {0};
    // 파일 앞부분에서 최대 3바이트를 읽어 실제로 읽힌 개수를 got 에 담는다
    size_t got = fread(bom, 1, 3, fp);
    // 읽은 3바이트가 UTF-8 BOM(EF BB BF)이 아니라면
    if (!(got == 3 && bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF)) {
        // BOM 이 아니므로 파일을 처음으로 되감아 그 바이트들도 데이터로 읽게 한다
        rewind(fp);
    }

    // CSV 한 줄을 담을 문자열 상자
    char line[256];
    // 실제로 저장에 성공한 강의 수
    int loaded = 0;
    // 지금 읽고 있는 줄 번호(오류 안내용)
    int line_no = 1;
    // 인코딩 경고를 이미 한 번 보여 줬는지 표시(중복 출력 방지)
    int warned_encoding = 0;

    // 첫 줄은 CSV 제목 행이므로 읽기만 하고 실제 강의로 저장하지 않는다
    if (fgets(line, sizeof(line), fp) == NULL) {
        // 첫 줄조차 없으면 빈 파일이므로 파일을 닫고
        fclose(fp);
        // 0개 로드 안내 후
        printf("[완료] CSV에서 강의 0개 로드 완료.\n");
        // 0을 돌려준다
        return 0;
    }

    // 둘째 줄부터 한 줄씩 읽되, 저장 공간이 가득 차면 멈춘다
    while (fgets(line, sizeof(line), fp) && course_count < MAX_COURSES) {
        // 줄 번호를 1 늘린다
        line_no++;

        // 빈 줄인지 확인하려고 포인터 p 를 줄 앞쪽 공백 뒤로 옮긴다
        const char *p = line;
        // 공백/탭/줄바꿈을 만나는 동안 한 칸씩 전진
        while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
        // 공백만 있던 빈 줄이면 건너뛴다
        if (*p == '\0') continue;

        // 아직 경고하지 않았고, 이 줄이 UTF-8 이 아니면
        if (!warned_encoding && !is_valid_utf8(line)) {
            // 한글이 깨질 수 있다는 경고와
            printf("\n[경고] 이 CSV 파일은 UTF-8이 아닌 것 같습니다. 한글이 깨질 수 있어요.\n");
            // 해결 방법을 안내하고
            printf("   Excel에서 '다른 이름으로 저장 → CSV UTF-8'로 다시 저장해 주세요.\n\n");
            // 다시 경고하지 않도록 표시한다
            warned_encoding = 1;
        }

        // strtok 은 line 을 쉼표(,) 기준으로 잘라 각 칸의 시작 주소를 돌려준다.
        // 두 번째 호출부터 NULL 을 넣는 것은 "같은 문자열에서 계속 자르라"는 뜻이다
        char *name_tok  = strtok(line, ",");
        // 둘째 칸: 교수명
        char *prof_tok  = strtok(NULL, ",");
        // 셋째 칸: 요일
        char *day_tok   = strtok(NULL, ",");
        // 넷째 칸: 시작 교시
        char *start_tok = strtok(NULL, ",");
        // 다섯째 칸: 종료 교시
        char *end_tok   = strtok(NULL, ",");
        // 여섯째 칸: 학점
        char *cred_tok  = strtok(NULL, ",");
        // 일곱째 칸: 평점
        char *rate_tok  = strtok(NULL, ",");
        // 여덟째 칸: 필수 여부(줄 끝의 \r\n 도 함께 잘라낸다)
        char *req_tok   = strtok(NULL, ",\r\n");

        // 여덟 칸 중 하나라도 비어 있으면(NULL) 형식이 어긋난 줄이다
        if (!name_tok || !prof_tok || !day_tok || !start_tok ||
            !end_tok || !cred_tok || !rate_tok || !req_tok) {
            // 칸이 부족하다고 안내하고 이 줄은 건너뛴다
            printf("  ※ %d번째 줄: 칸 수가 부족해 건너뜁니다.\n", line_no);
            continue;
        }

        // 각 칸의 앞뒤 공백을 trim 으로 없앤다. 글자 칸은 그대로, 숫자 칸은 숫자로 바꾼다
        char *name     = trim(name_tok);
        // 교수명 앞뒤 공백 제거
        char *prof     = trim(prof_tok);
        // 요일 문자열 앞뒤 공백 제거
        char *days     = trim(day_tok);
        // 시작 교시 문자열을 정수로 바꾼다(atoi)
        int   start    = atoi(trim(start_tok));
        // 종료 교시 문자열을 정수로 바꾼다
        int   end      = atoi(trim(end_tok));
        // 학점 문자열을 정수로 바꾼다
        int   credit   = atoi(trim(cred_tok));
        // 평점 문자열을 실수로 바꾼다(atof)
        float rating   = (float)atof(trim(rate_tok));
        // 필수 여부 문자열을 정수로 바꾼다
        int   required = atoi(trim(req_tok));

        // 강의명이 비어 있으면 쓸 수 없는 줄이므로
        if (name[0] == '\0') {
            // 안내 후 건너뛴다
            printf("  ※ %d번째 줄: 강의명이 비어 건너뜁니다.\n", line_no);
            continue;
        }
        // 교시 값이 1~9 범위를 벗어나거나 시작이 종료보다 뒤이면 잘못된 값이다
        if (start < 1 || start > MAX_PERIOD || end < 1 || end > MAX_PERIOD || start > end) {
            // 안내 후 건너뛴다
            printf("  ※ %d번째 줄(%s): 교시 값이 올바르지 않아 건너뜁니다.\n", line_no, name);
            continue;
        }
        // 학점이 1~30 범위를 벗어나면 잘못된 값이다
        if (credit < 1 || credit > 30) {
            // 안내 후 건너뛴다
            printf("  ※ %d번째 줄(%s): 학점 값이 올바르지 않아 건너뜁니다.\n", line_no, name);
            continue;
        }
        // 평점이 0.0~5.0 범위를 벗어나면 잘못된 값이다
        if (rating < 0.0f || rating > 5.0f) {
            // 안내 후 건너뛴다
            printf("  ※ %d번째 줄(%s): 평점 값이 올바르지 않아 건너뜁니다.\n", line_no, name);
            continue;
        }

        // 필수 여부는 1이 아니면 모두 0(선택)으로 정리한다
        required = (required == 1) ? 1 : 0;

        // 모든 검증을 통과했으니, 다음 빈 칸에 이 강의를 저장한다
        Course *c = &course_list[course_count];
        // 칸을 0으로 깨끗이 비우고
        memset(c, 0, sizeof(Course));
        // 요일 칸을 -1(없음)로 초기화한다
        for (int d = 0; d < MAX_DAYS; d++) c->days[d] = -1;

        // 강의명을 복사한다(칸 크기를 넘지 않게 -1 까지만)
        strncpy(c->name, name, MAX_NAME_LEN - 1);
        // 교수명을 복사한다
        strncpy(c->professor, prof, MAX_PROF_LEN - 1);
        // 시작 교시를 넣는다
        c->start_period = start;
        // 종료 교시를 넣는다
        c->end_period   = end;
        // 학점을 넣는다
        c->credit       = credit;
        // 평점을 넣는다
        c->rating       = rating;
        // 필수 여부를 넣는다
        c->is_required  = required;

        // 요일 문자열을 숫자 배열로 바꿔 저장한다
        parse_days_into(c, days);
        // 요일을 하나도 알아보지 못했다면
        if (c->day_count == 0) {
            // 안내하고
            printf("  ※ %d번째 줄(%s): 요일을 알아보지 못해 월요일로 설정합니다.\n",
                   line_no, name);
            // 기본값으로 월요일 하나를 넣는다
            c->days[0] = MON;
            c->day_count = 1;
        }

        // 저장된 전체 강의 수를 1 늘린다
        course_count++;
        // 이번 호출에서 읽어 들인 수도 1 늘린다
        loaded++;
    }

    // 파일을 닫는다(열었으면 반드시 닫아 준다)
    fclose(fp);
    // 몇 개를 읽었는지 알려 준다
    printf("[완료] CSV에서 강의 %d개 로드 완료.\n", loaded);
    // 읽은 개수를 돌려준다
    return loaded;
}

/*
 * 현재 저장된 강의 목록을 표 형태로 출력합니다.
 * 먼저 각 열에 필요한 폭을 계산한 뒤, 제목/구분선/데이터 행을 차례로 출력합니다.
 */
void print_course_list(void) {
    // 각 열의 최소 폭(글자 폭이 더 크면 그만큼 넓어진다)
    int no_width = 4;
    // 강의명 열의 시작 폭
    int name_width = 20;
    // 교수명 열의 시작 폭
    int professor_width = 12;
    // 요일 열의 시작 폭
    int day_width = 8;
    // 숫자(교시·학점·평점) 열의 폭
    int number_width = 5;
    // 필수 여부 열의 시작 폭
    int required_width = 6;

    // 1단계: 모든 강의를 훑으며 각 열에 필요한 최대 폭을 구한다
    for (int i = 0; i < course_count; i++) {
        // i 번째 강의의 주소
        Course *c = &course_list[i];

        // 이 강의의 요일들을 "월수금" 형태 문자열로 만든다
        char day_str[20];
        print_course_days(c, day_str, sizeof(day_str));

        // 강의명 폭이 더 길면 열 폭을 그만큼 늘린다
        name_width = max_int(name_width, display_width(c->name));
        // 교수명도 마찬가지로 가장 긴 것에 맞춘다
        professor_width = max_int(professor_width, display_width(c->professor));
        // 요일 문자열 폭도 가장 긴 것에 맞춘다
        day_width = max_int(day_width, display_width(day_str));
        // "필수"/"선택" 글자 폭에 맞춘다
        required_width = max_int(required_width,
                                 display_width(c->is_required ? "필수" : "선택"));
    }

    // 2단계: 표의 제목 줄을 출력한다
    printf("\n");
    // "No" 칸(왼쪽 정렬) 뒤에 칸 사이 공백 한 칸
    print_cell("No", no_width, 0);           putchar(' ');
    // "강의명" 칸
    print_cell("강의명", name_width, 0);     putchar(' ');
    // "교수명" 칸
    print_cell("교수명", professor_width, 0); putchar(' ');
    // "요일" 칸
    print_cell("요일", day_width, 0);        putchar(' ');
    // "시작" 칸(숫자라 오른쪽 정렬)
    print_cell("시작", number_width, 1);     putchar(' ');
    // "종료" 칸
    print_cell("종료", number_width, 1);     putchar(' ');
    // "학점" 칸
    print_cell("학점", number_width, 1);     putchar(' ');
    // "평점" 칸
    print_cell("평점", number_width, 1);     putchar(' ');
    // "필수" 칸을 출력하고 줄을 바꾼다
    print_cell("필수", required_width, 1);   putchar('\n');

    // 제목 아래 구분선을 열 폭에 맞춰 그린다
    print_rule_cell(no_width);        putchar(' ');
    print_rule_cell(name_width);      putchar(' ');
    print_rule_cell(professor_width); putchar(' ');
    print_rule_cell(day_width);       putchar(' ');
    print_rule_cell(number_width);    putchar(' ');
    print_rule_cell(number_width);    putchar(' ');
    print_rule_cell(number_width);    putchar(' ');
    print_rule_cell(number_width);    putchar(' ');
    print_rule_cell(required_width);  putchar('\n');

    // 3단계: 각 강의를 한 줄씩 출력한다. 숫자는 문자열로 바꾼 뒤 칸에 맞춘다
    for (int i = 0; i < course_count; i++) {
        // i 번째 강의의 주소
        Course *c = &course_list[i];

        // 요일 문자열을 담을 상자
        char day_str[20];
        // 번호(No)를 담을 상자
        char no_text[16];
        // 시작 교시를 담을 상자
        char start_text[16];
        // 종료 교시를 담을 상자
        char end_text[16];
        // 학점을 담을 상자
        char credit_text[16];
        // 평점을 담을 상자
        char rating_text[16];

        // 요일들을 "월수금" 형태로 만든다
        print_course_days(c, day_str, sizeof(day_str));
        // 번호(1부터 시작)를 글자로 바꾼다
        snprintf(no_text, sizeof(no_text), "%d", i + 1);
        // 시작 교시를 글자로 바꾼다
        snprintf(start_text, sizeof(start_text), "%d", c->start_period);
        // 종료 교시를 글자로 바꾼다
        snprintf(end_text, sizeof(end_text), "%d", c->end_period);
        // 학점을 글자로 바꾼다
        snprintf(credit_text, sizeof(credit_text), "%d", c->credit);
        // 평점을 소수 첫째 자리까지 글자로 바꾼다
        snprintf(rating_text, sizeof(rating_text), "%.1f", c->rating);

        // 번호 칸 출력
        print_cell(no_text, no_width, 0);                    putchar(' ');
        // 강의명 칸 출력
        print_cell(c->name, name_width, 0);                  putchar(' ');
        // 교수명 칸 출력
        print_cell(c->professor, professor_width, 0);        putchar(' ');
        // 요일 칸 출력
        print_cell(day_str, day_width, 0);                   putchar(' ');
        // 시작 교시 칸 출력(오른쪽 정렬)
        print_cell(start_text, number_width, 1);             putchar(' ');
        // 종료 교시 칸 출력
        print_cell(end_text, number_width, 1);               putchar(' ');
        // 학점 칸 출력
        print_cell(credit_text, number_width, 1);            putchar(' ');
        // 평점 칸 출력
        print_cell(rating_text, number_width, 1);            putchar(' ');
        // 필수/선택 칸 출력
        print_cell(c->is_required ? "필수" : "선택", required_width, 1);
        // 한 강의 줄을 마치고 줄을 바꾼다
        putchar('\n');
    }
}
