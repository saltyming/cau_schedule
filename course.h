#ifndef COURSE_H
#define COURSE_H

#define MAX_COURSES     100
#define MAX_NAME_LEN    60
#define MAX_PROF_LEN    40
#define MAX_DAYS        3
#define DAYS_IN_WEEK    5
#define MAX_PERIOD      9

#define MON 0
#define TUE 1
#define WED 2
#define THU 3
#define FRI 4

typedef struct {
    char  name[MAX_NAME_LEN];
    char  professor[MAX_PROF_LEN];
    int   days[MAX_DAYS];
    int   day_count;
    int   start_period;
    int   end_period;
    float rating;
    int   credit;
    int   is_required;
} Course;

extern Course course_list[MAX_COURSES];
extern int    course_count;

void  input_courses_manual(void);
int   load_courses_from_csv(const char *filename);
void  print_course_list(void);
int   parse_day(const char *s);
const char *day_to_str(int d);
int   read_int(const char *prompt, int min_value, int max_value);
float read_float(const char *prompt, float min_value, float max_value);
void  read_line(const char *prompt, char *buf, int buf_size);

#endif
