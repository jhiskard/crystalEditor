# P9-BUG-01 재현 로그 (W0 Restart Baseline)

등록일: `2026-04-02 (KST)`  
이슈 ID: `P9-BUG-01`  
연계 문서: `docs/refactoring/phase8/go_no_go_phase9.md`

## 1. 증상
- `XSF(Grid) -> VASP -> XSF(Grid) -> VASP` 순서로 파일을 로드하면,
  이전 grid 데이터가 완전히 제거되지 않고 잔존하는 현상이 발생한다.

## 2. 재현 절차 (W0 고정)
1. 페이지 새로고침 후 XSF(Grid 포함) 파일을 로드한다.
2. VASP 파일을 로드한다.
3. 다시 XSF(Grid 포함) 파일을 로드한다.
4. 다시 VASP 파일을 로드한다.
5. Model Tree 및 Viewer에서 grid/isovalue 관련 잔존 표현 여부를 확인한다.

## 3. 기대 결과 / 실제 결과
- 기대 결과:
  - VASP 로드 시 기존 XSF grid 데이터가 완전히 정리되어야 한다.
  - 이후 VASP 기준 구조/표시 상태만 남아야 한다.
- 실제 결과:
  - 특정 순서에서 기존 grid 표시/상태가 남아 후속 표시와 중첩될 수 있다.

## 4. 현재 상태
- 상태: `Open`
- 우선순위: `Medium`
- 비고:
  - 기존 수정 시도 2건은 롤백됨.
  - Phase 9 W4에서 원인 귀속(render/import/workspace) 후 수정 또는 이관 판정 예정.

## 5. Phase 9 처리 규칙
1. W0에서 재현 절차와 관찰 포인트를 고정한다.
2. W3~W4에서 clear/apply/rollback 및 actor detach 흐름을 점검한다.
3. W6 종료 문서(`dependency_gate_report.md`, `go_no_go_phase10.md`)에 해결/이관 상태를 명시한다.

## 6. W3 점검 메모 (2026-04-02 KST)
- W3 범위에서 `render` 경계 전환(포트/어댑터 경유화)을 우선 적용했다.
- 현재 상태:
  - `P9-BUG-01`: `Open` (해결 전)
  - 원인 귀속 후보: import apply/rollback 순서 또는 workspace/grid 컨텍스트 clear 누락
- W4에서 확인할 포인트:
  1. VASP 로딩 직전/직후 grid actor detach 누락 여부
  2. import 실패/재시도 경로의 rollback 이후 상태 잔존 여부
  3. 구조 전환 시 density context reset 순서
