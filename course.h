#ifndef COURSE_H
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

/* 배열 크기와 요일 번호를 코드 곳곳에서 같은 의미로 쓰기 위한 상수입니다. */
#define MAX_COURSES     100  /* 프로그램이 저장할 수 있는 최대 강의 수 */
#define MAX_NAME_LEN    60   /* 강의명 문자열 배열 크기 */
#define MAX_PROF_LEN    40   /* 교수명 문자열 배열 크기 */
#define MAX_DAYS        3    /* 한 강의가 가질 수 있는 최대 요일 수(예: 월수금) */
#define DAYS_IN_WEEK    5    /* 월~금만 다루므로 5일 */
#define MAX_PERIOD      9    /* 하루 최대 교시 */

/* 요일을 문자열 대신 숫자로 저장합니다. 배열 인덱스로 쓰기 쉽기 때문입니다. */
#define MON 0
#define TUE 1
#define WED 2
#define THU 3
#define FRI 4

/*
 * struct는 여러 값을 하나의 묶음으로 다루는 C 문법입니다.
 * Course 하나에는 강의명, 교수명, 수업 요일, 교시, 평점, 학점 등이 함께 들어갑니다.
 */
typedef struct {
    char  name[MAX_NAME_LEN];       /* 문자열은 char 배열로 저장합니다. */
    char  professor[MAX_PROF_LEN];  /* 교수명도 고정 크기 char 배열입니다. */
    int   days[MAX_DAYS];           /* 수업 요일 번호들: MON, WED 같은 값이 들어갑니다. */
    int   day_count;                /* days 배열에 실제로 몇 개의 요일이 들어 있는지 */
    int   start_period;             /* 시작 교시 */
    int   end_period;               /* 종료 교시 */
    float rating;                   /* 강의 평점 */
    int   credit;                   /* 학점 */
    int   is_required;              /* 필수 과목이면 1, 선택 과목이면 0 */
} Course;

/*
 * extern은 "이 변수는 다른 .c 파일에 실제로 만들어져 있다"는 뜻입니다.
 * 실제 저장 공간은 course.c에 있고, 다른 파일들은 이 선언을 보고 함께 사용합니다.
 */
extern Course course_list[MAX_COURSES];
extern int    course_count;

/* 강의 데이터 입력, CSV 로딩, 목록 출력에 쓰이는 함수 선언들입니다. */
void  input_courses_manual(void);
int   load_courses_from_csv(const char *filename);
void  print_course_list(void);
int   parse_day(const char *s);
const char *day_to_str(int d);
int   read_int(const char *prompt, int min_value, int max_value);
float read_float(const char *prompt, float min_value, float max_value);
void  read_line(const char *prompt, char *buf, int buf_size);

#endif
