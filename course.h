#ifndef COURSE_H
#define COURSE_H

// MARK: - 용량 / 범위 상수
// 프로그램이 다루는 한계값들을 한곳에 모아 둡니다.
// 숫자를 바꾸고 싶으면 여기만 고치면 됩니다.

#define MAX_COURSES     100     // 한 번에 다룰 수 있는 최대 강의 수
#define MAX_NAME_LEN    60      // 강의명 최대 길이(바이트). 한글 1글자 = 3바이트(UTF-8)
#define MAX_PROF_LEN    40      // 교수명 최대 길이(바이트)
#define MAX_DAYS        3       // 한 강의의 최대 요일 수 (예: "월수금" → 3일)
#define DAYS_IN_WEEK    5       // 한 주에서 다루는 요일 수 (월~금)
#define MAX_PERIOD      9       // 하루의 최대 교시 번호 (1~9교시)

// MARK: - 요일 인덱스
// 요일을 0~4 숫자로 다루면 배열의 칸 번호로 그대로 쓸 수 있어 편리합니다.

#define MON 0   // 월요일
#define TUE 1   // 화요일
#define WED 2   // 수요일
#define THU 3   // 목요일
#define FRI 4   // 금요일

// MARK: - 강의 한 개를 표현하는 자료형

/**
 강의 한 개의 모든 정보를 담는 상자(구조체)입니다.

 CSV 파일의 한 줄, 또는 사용자가 직접 입력한 강의 하나가
 이 구조체 하나에 그대로 대응됩니다.
 */
typedef struct {
    /// 강의명입니다. 예) "컴퓨터구조"
    char  name[MAX_NAME_LEN];

    /// 교수명입니다. 예) "김철수"
    char  professor[MAX_PROF_LEN];

    /// 수업하는 요일 목록입니다. 0=월 … 4=금이며, 빈 칸은 -1로 둡니다.
    int   days[MAX_DAYS];

    /// days 배열에 실제로 채워진 요일 수입니다. 예) "월수" → 2
    int   day_count;

    /// 시작 교시입니다. (1~9)
    int   start_period;

    /// 종료 교시입니다. (1~9) 시작 교시와 같으면 한 교시짜리 수업입니다.
    int   end_period;

    /// 강의 평점입니다. (0.0~5.0) 숫자가 클수록 평이 좋은 강의입니다.
    float rating;

    /// 학점 수입니다.
    int   credit;

    /// 1이면 반드시 들어야 하는 필수 과목, 0이면 선택 과목입니다.
    int   is_required;
} Course;

// MARK: - 전역 강의 목록
// 입력된 모든 강의는 아래 한 배열에 모이고, 프로그램의 어느 파일에서나
// 같은 목록을 함께 바라봅니다.

/// 입력된 모든 강의가 저장되는 전역 배열입니다.
extern Course course_list[MAX_COURSES];

/// course_list에 실제로 채워진 강의 개수입니다.
extern int    course_count;

// MARK: - 함수 선언

/**
 사용자에게 강의 정보를 직접 입력받아 course_list에 추가합니다.

 먼저 입력할 강의 수를 묻고, 강의마다 이름·요일·교시 등을
 차례대로 물어봅니다.
 */
void  input_courses_manual(void);

/**
 CSV 파일을 읽어 강의들을 course_list에 추가합니다.

 파일의 첫 줄은 제목 줄로 보고 건너뜁니다. 각 줄의 형식은 다음과 같습니다.
 강의명,교수명,요일,시작교시,종료교시,학점,평점,필수여부

 파일은 반드시 UTF-8로 저장되어 있어야 합니다. (Excel: "다른 이름으로 저장
 → CSV UTF-8") 형식이 잘못된 줄은 건너뛰고 경고만 보여 줍니다.

 - Parameter filename: 읽어들일 CSV 파일 경로
 - Returns: 새로 불러온 강의 수. 파일을 열 수 없으면 -1.
 */
int   load_courses_from_csv(const char *filename);

/**
 현재까지 입력된 모든 강의를 표 형태로 화면에 보여 줍니다.
 */
void  print_course_list(void);

/**
 요일 글자 하나를 숫자 인덱스로 바꿉니다. 예) "월" → 0(MON)

 - Parameter s: 요일이 들어 있는 문자열
 - Returns: 0~4 사이의 요일 인덱스. 알아보지 못하면 -1.
 */
int   parse_day(const char *s);

/**
 요일 인덱스를 사람이 읽는 글자로 바꿉니다. 예) 0 → "월"

 - Parameter d: 0~4 사이의 요일 인덱스
 - Returns: "월"~"금" 문자열. 범위를 벗어나면 "?".
 */
const char *day_to_str(int d);

// MARK: - 콘솔 입력 도우미
// 사용자가 잘못된 값을 입력해도 프로그램이 멈추거나 엉뚱하게 동작하지 않도록,
// 올바른 값이 들어올 때까지 친절하게 다시 물어보는 안전한 입력 함수들입니다.

/**
 정수를 입력받습니다. 숫자가 아니거나 허용 범위를 벗어나면
 안내 문구를 보여 준 뒤 다시 물어봅니다.

 - Parameter prompt: 사용자에게 보여 줄 안내 문구
 - Parameter min_value: 허용하는 가장 작은 값
 - Parameter max_value: 허용하는 가장 큰 값
 - Returns: min_value~max_value 범위의 정수. 입력이 끝나면(EOF) min_value.
 */
int   read_int(const char *prompt, int min_value, int max_value);

/**
 실수(소수점이 있는 숫자)를 입력받습니다. 범위를 벗어나면 다시 물어봅니다.

 - Parameter prompt: 안내 문구
 - Parameter min_value: 허용하는 가장 작은 값
 - Parameter max_value: 허용하는 가장 큰 값
 - Returns: min_value~max_value 범위의 실수. 입력이 끝나면(EOF) min_value.
 */
float read_float(const char *prompt, float min_value, float max_value);

/**
 한 줄을 입력받아 buf에 담습니다. 줄 끝의 줄바꿈 문자는 없애 줍니다.

 - Parameter prompt: 안내 문구
 - Parameter buf: 입력한 글자를 담을 공간
 - Parameter buf_size: buf의 크기(바이트)
 */
void  read_line(const char *prompt, char *buf, int buf_size);

#endif
