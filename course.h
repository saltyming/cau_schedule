// 헤더 중복 포함 방지: COURSE_H 가 아직 정의되지 않았을 때만 아래 내용을 포함한다
#ifndef COURSE_H
// COURSE_H 라는 이름표를 정의해, 이 파일 내용이 두 번 포함되지 않도록 표시한다
#define COURSE_H

/*
 * course.h
 * --------
 * 강의(Course) 데이터를 표현하는 구조체와, 강의 입력/출력에 필요한 함수들의
 * 약속(선언)을 모아 둔 헤더 파일입니다.
 *
 * C에서는 다른 .c 파일에 있는 함수를 사용하려면, 함수 이름과 매개변수 형태를
 * 미리 알려 주어야 합니다. 이 파일의 함수 선언들이 그 역할을 합니다.
 */

// 아래는 배열 크기와 요일 번호를 코드 곳곳에서 같은 의미로 쓰기 위한 상수들이다

// 프로그램이 저장할 수 있는 최대 강의 수(이 숫자만큼 강의를 담을 수 있다)
#define MAX_COURSES     100
// 강의명을 담는 문자열(char 배열)의 칸 수
#define MAX_NAME_LEN    60
// 교수명을 담는 문자열의 칸 수
#define MAX_PROF_LEN    40
// 한 강의가 가질 수 있는 최대 요일 수(예: 월수금이면 3개)
#define MAX_DAYS        3
// 이 프로그램은 월~금만 다루므로 일주일을 5일로 본다
#define DAYS_IN_WEEK    5
// 하루에 가능한 최대 교시 번호
#define MAX_PERIOD      9

// 요일을 글자 대신 숫자로 저장한다(배열의 몇 번째 칸인지로 쓰기 편하기 때문)
// 월요일을 0번으로 정한다
#define MON 0
// 화요일을 1번으로 정한다
#define TUE 1
// 수요일을 2번으로 정한다
#define WED 2
// 목요일을 3번으로 정한다
#define THU 3
// 금요일을 4번으로 정한다
#define FRI 4

/*
 * struct는 여러 값을 하나의 묶음으로 다루는 C 문법입니다.
 * Course 하나에는 강의명, 교수명, 수업 요일, 교시, 평점, 학점 등이 함께 들어갑니다.
 */
// typedef struct { ... } Course; 는 "이 묶음 자료형을 Course 라는 이름으로 부르겠다"는 뜻
typedef struct {
    // 강의 이름을 저장하는 문자열(예: "자료구조")
    char  name[MAX_NAME_LEN];
    // 교수 이름을 저장하는 문자열
    char  professor[MAX_PROF_LEN];
    // 수업하는 요일 번호들을 담는 배열(예: 월·수·금이면 {0, 2, 4})
    int   days[MAX_DAYS];
    // days 배열에 실제로 몇 개의 요일이 들어 있는지(예: 월수금이면 3)
    int   day_count;
    // 수업이 시작하는 교시
    int   start_period;
    // 수업이 끝나는 교시
    int   end_period;
    // 강의 평점(소수점이 있을 수 있어 실수형 float 으로 저장)
    float rating;
    // 이 강의의 학점 수
    int   credit;
    // 반드시 들어야 하는 필수 과목이면 1, 아니면(선택 과목) 0
    int   is_required;
} Course;

/*
 * extern은 "이 변수는 다른 .c 파일에 실제로 만들어져 있다"는 뜻입니다.
 * 실제 저장 공간은 course.c에 있고, 다른 파일들은 이 선언을 보고 함께 사용합니다.
 */
// 모든 강의를 담아 두는 전역 배열(실제 알맹이는 course.c 에 만들어져 있다)
extern Course course_list[MAX_COURSES];
// 현재 course_list 에 담겨 있는 강의의 개수
extern int    course_count;

// 아래는 강의 입력·CSV 읽기·목록 출력에 쓰이는 함수들의 선언(약속)이다

// 사용자가 키보드로 강의 정보를 직접 입력하게 한다
void  input_courses_manual(void);
// CSV 파일에서 강의들을 읽어 들인다(성공하면 읽은 개수를 돌려준다)
int   load_courses_from_csv(const char *filename);
// 지금까지 담긴 강의 목록을 표 형태로 화면에 보여 준다
void  print_course_list(void);
// "월","화"... 같은 글자가 들어 있는 문자열에서 요일 번호를 찾아낸다
int   parse_day(const char *s);
// 요일 번호(0~4)를 "월"~"금" 글자로 바꿔서 돌려준다
const char *day_to_str(int d);
// 화면에 prompt 를 보여 주고, min~max 범위의 정수를 안전하게 입력받는다
int   read_int(const char *prompt, int min_value, int max_value);
// 화면에 prompt 를 보여 주고, min~max 범위의 실수를 안전하게 입력받는다
float read_float(const char *prompt, float min_value, float max_value);
// 화면에 prompt 를 보여 주고, 한 줄의 글자를 입력받아 buf 에 담는다
void  read_line(const char *prompt, char *buf, int buf_size);

// #ifndef COURSE_H 로 시작한 중복 포함 방지 블록을 여기서 닫는다
#endif
