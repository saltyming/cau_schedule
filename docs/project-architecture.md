# 전체 모듈 구조

프로젝트는 `main.c`가 실행 흐름을 잡고, 나머지 모듈이 입력, 검증, 최적화, 출력, 저장 역할을 나눠 맡는 구조입니다.

```mermaid
flowchart LR
  main["main.c<br/>메뉴와 전체 실행 흐름"]
  course["course.c / course.h<br/>강의 입력, CSV 로드, 강의 목록 출력"]
  schedule["schedule.c / schedule.h<br/>시간 충돌 검사, 시간표 조회, 통계 계산"]
  optimizer["optimizer.c / optimizer.h<br/>모드별 시간표 최적화"]
  display["display.c / display.h<br/>콘솔 요약 출력"]
  export["export.c / export.h<br/>CSV 결과 저장"]

  main --> course
  main --> schedule
  main --> optimizer
  main --> display
  main --> export

  optimizer --> schedule
  optimizer --> course

  schedule --> course

  display --> schedule
  display --> course

  export --> schedule
  export --> course
```

## 핵심 연결

- `main.c`는 사용자 선택을 받아 각 기능 모듈을 호출합니다.
- `course.c`는 전역 `course_list`와 `course_count`를 관리합니다.
- `optimizer.c`는 `Schedule`을 만들고, `schedule.c`의 검사 함수를 사용합니다.
- `display.c`와 `export.c`는 만들어진 `Schedule`을 읽어 화면과 CSV로 표현합니다.

