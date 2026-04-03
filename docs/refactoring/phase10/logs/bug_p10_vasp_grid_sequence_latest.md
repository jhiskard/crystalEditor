# P9-BUG-01 Tracking Log (Phase 10)

- Bug ID: `P9-BUG-01`
- Scenario: `XSF(Grid) -> VASP -> XSF(Grid) -> VASP`
- Last updated: `2026-04-03 (KST)`
- Status: `Deferred`

## 1) Baseline

- Source baseline: `docs/refactoring/phase9/logs/bug_p9_vasp_grid_sequence_latest.md`
- Known symptom:
  - VASP 로드 이후 기존 grid 컨텍스트/표시 상태가 완전히 정리되지 않을 수 있음
  - 반복 시퀀스에서 stale grid 데이터가 남는 경우가 있음

## 2) W0~W4 범위에서의 처리

1. W0: 재현 시퀀스를 Phase 10 추적 대상으로 고정했다.
2. W1: `workspace` 문맥 저장소를 추가해 구조/메시/밀도 active id를 단일 소스로 관리할 기반을 마련했다.
3. W2: mesh 접근 경로 일부를 repository/service 경유로 전환해 import orchestration 경계 분리를 시작했다.
4. W3: `ImportWorkflowService`를 도입해 replace-scene transaction(`Begin/Finalize/Rollback/Cleanup`)을 `FileLoader`에서 분리했다.
5. W4: `ModelTree` selected mesh id를 `workspace`에 동기화하고 panel 경로 singleton 의존을 추가 축소했다.

## 3) 현재 판정

- `Deferred` 유지
- 사유:
  - W3까지 workflow 경계 분리와 context 동기화 기반은 도입했지만,
    `import_apply_service`/`mesh_detail`/`atoms_template` 잔여 경로의 clear 순서 통합이 남아 있다.
  - 반복 시퀀스 동적 검증은 W6 게이트에서 최종 확정한다.

## 4) 다음 액션

1. W3: `FileLoader` -> `import_workflow_service` 축소 과정에서 clear/finalize/rollback 순서 재검증
2. W4: panel/read-model 전환과 함께 stale grid 컨텍스트 제거 시점 고정
3. W6: `XSF(Grid) -> VASP -> XSF(Grid) -> VASP` 시퀀스의 smoke/manual 결과로 `Resolved` 또는 `Deferred` 최종 판정
