# `main()` 메뉴 흐름

`main()`은 프로그램 시작 후 메뉴를 반복해서 보여 주고, 선택 번호에 따라 입력, 조회, 최적화, 종료를 실행합니다.

```mermaid
flowchart TD
  A["main()"] --> B["print_banner()"]
  B --> C["do-while 메뉴 루프"]
  C --> D["print_menu()"]
  D --> E["read_int('선택 > ', 0, 8)"]
  E --> F{"choice"}

  F -->|1| G["input_courses_manual()"]
  F -->|2| H["read_line(CSV 파일명)"]
  H --> I["load_courses_from_csv(filename)"]
  F -->|3| J["create_sample_csv()"]
  J --> K["load_courses_from_csv('sample_courses.csv')"]
  F -->|4| L{"course_count == 0?"}
  L -->|예| M["입력된 강의 없음 안내"]
  L -->|아니오| N["print_course_list()"]

  F -->|5, 6, 7| O{"course_count == 0?"}
  O -->|예| P["먼저 강의 입력/로드 안내"]
  O -->|아니오| Q["read_int(최대 학점)"]
  Q --> R["run_single_mode(choice - 4, max_credit)"]

  F -->|8| S{"course_count == 0?"}
  S -->|예| T["먼저 강의 입력/로드 안내"]
  S -->|아니오| U["read_int(최대 학점)"]
  U --> V["run_all_modes(max_credit)"]

  F -->|0| W["종료 문구 출력"]

  G --> C
  I --> C
  K --> C
  M --> C
  N --> C
  P --> C
  R --> C
  T --> C
  V --> C
  W --> X["return 0"]
```

## 선택 번호별 역할

- `1`: 콘솔에서 강의를 직접 입력합니다.
- `2`: 사용자가 지정한 CSV 파일에서 강의를 불러옵니다.
- `3`: 샘플 CSV를 만든 뒤 바로 불러옵니다.
- `4`: 현재 강의 목록을 출력합니다.
- `5`-`7`: 모드 하나를 선택해 시간표를 생성합니다.
- `8`: 세 가지 모드를 모두 생성합니다.
- `0`: 프로그램을 종료합니다.
