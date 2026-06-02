# 구조도 문서

이 디렉터리는 프로젝트 실행 흐름과 모듈 연결 구조를 Mermaid 다이어그램으로 정리합니다.

## 문서 목록

- [project-architecture.md](project-architecture.md): 전체 모듈 의존 관계
- [main-menu-flow.md](main-menu-flow.md): `main()`에서 시작되는 메뉴 처리 흐름
- [optimizer-flow.md](optimizer-flow.md): 시간표 최적화와 백트래킹 흐름
- [data-and-output-flow.md](data-and-output-flow.md): 강의 데이터, 시간표 데이터, 출력 파일 흐름
- [build-flow.md](build-flow.md): `Makefile` 기준 빌드 구조

## 보는 방법

아래 중 하나를 사용하면 Mermaid 구조도를 그림으로 볼 수 있습니다.

- GitHub에 Markdown 파일을 올려서 보기
- VS Code에서 Mermaid 지원 Markdown Preview 확장으로 보기
- <https://mermaid.live>에 Mermaid 코드 블록 내용 붙여 넣기
- Mermaid CLI로 이미지 변환하려면 Mermaid 코드 블록만 별도 `.mmd` 파일로 저장한 뒤 실행:

```bash
mmdc -i project-architecture.mmd -o project-architecture.png
```
