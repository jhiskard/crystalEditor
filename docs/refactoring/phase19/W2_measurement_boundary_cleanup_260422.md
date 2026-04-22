# Phase 19 / W2 — Measurement 경계 정리 (`vtkActor*` 제거)

작성일: `2026-04-22 (KST)`
상위 계획서: `docs/refactoring/refactoring_plan_phase19_legacy_complete_dismantle_codex_260421_v2.md`
검증보고서 근거: 5.1 경계 위반 잔존(Measurement), 5.2 Medium 권고
우선순위: **Medium**
예상 소요: 0.5 ~ 1일
선행 의존: W0
후속 작업: W5.2 (Measurement 로직 본체 이관), W9 (Phase19 게이트 편입)

---

## 1. 배경

Phase 15 이후에도 `measurement/application` 계층에 `vtkActor*` 토큰이 남아 있으면 render 경계가 누출된 상태다. W2는 Measurement 입력을 값 객체(`PickedAtomInfo`)로 전환해 render 계층 외부의 VTK 의존을 제거한다.

## 2. 현재 상태

| 항목 | 값 |
|---|---|
| `measurement/application` 하위 `vtkActor*` 참조 | 1건 |
| 대표 위치 | `measurement_controller.cpp`의 picker 클릭 핸들러 |
| 호출 출발점 | `render/application/viewer_interaction_controller.cpp` |
| 목표 인터페이스 | `RenderGateway::ResolvePickedAtom(...)` + 값 객체 전달 |

## 3. 설계

### 3.1 값 객체 도입

`render/application/picked_atom_info.h`를 추가하고 최소 필드를 포함한다.

- `hit`
- `structureId`
- `atomId`
- `pickPosition`
- `atomPosition`

### 3.2 경계 재정의

- render: picker 결과를 `PickedAtomInfo`로 변환
- measurement: 값 객체만 수신
- 규칙: `measurement/**`에서 `vtk*` 타입/헤더 0

### 3.3 단계 분리

W2는 "VTK 토큰 제거"에 한정한다. Measurement 동작 본체의 완전 분해는 W5.2에서 수행한다.

## 4. 실행 순서

1. `measurement/**`의 `vtk*` 토큰/헤더 인벤토리 기록.
2. `PickedAtomInfo` 헤더 추가.
3. `RenderGateway`에 picked atom 해결 API 추가/정비.
4. `MeasurementService` 서명을 값 객체 기반으로 전환.
5. 호출부(`viewer_interaction_controller`)를 값 전달 흐름으로 갱신.
6. `measurement/**`의 VTK include 제거.
7. `check_phase15_render_platform_isolation.ps1` 재실행.
8. W2 완료 로그 작성.

## 5. 파일 변경 명세

| 경로 | 변경 유형 | 비고 |
|---|---|---|
| `webassembly/src/render/application/picked_atom_info.h` | **신규** | 값 객체 |
| `webassembly/src/render/application/render_gateway.h` | 수정 | resolve API |
| `webassembly/src/render/infrastructure/vtk_render_gateway.h` | 수정 | 구현 선언 |
| `webassembly/src/render/infrastructure/vtk_render_gateway.cpp` | 수정 | 구현 |
| `webassembly/src/measurement/application/measurement_service.h` | 수정 | 서명 변경 |
| `webassembly/src/measurement/application/measurement_service.cpp` | 수정 | 구현 변경 |
| `webassembly/src/measurement/application/measurement_controller.cpp` | 수정 | `vtkActor*` 제거 |
| `webassembly/src/render/application/viewer_interaction_controller.cpp` | 수정 | 호출부 갱신 |
| `docs/refactoring/phase19/logs/measurement_vtk_token_inventory_phase19_latest.md` | **신규** | 인벤토리 |
| `docs/refactoring/phase19/logs/dependency_gate_report_W2.md` | **신규** | 완료 리포트 |

## 6. 완료 기준 (DoD)

| 항목 | 측정 | 기준 |
|---|---|---|
| `measurement/**` 내 `vtkActor`/`vtkVolume`/`vtkSmartPointer` | grep | 0 |
| `measurement/**` 내 `#include <vtk...>` | grep | 0 |
| `check_phase15_render_platform_isolation.ps1` | 스크립트 실행 | PASS |
| Measurement 클릭/선택 회귀 | e2e/수동 | PASS |

## 7. 리스크와 완화

| 리스크 | 영향 | 완화책 |
|---|---|---|
| 값 객체 필드 부족 | 측정 로직 불완전 | 기존 핸들러에서 참조하는 데이터를 먼저 전수 매핑 |
| render/measurement 서명 동시 변경 충돌 | 빌드 실패 | 한 커밋에서 인터페이스/호출부 동시 갱신 |
| W5.2와 경계 중복 변경 | 재작업 | W2는 경계 계약만 확정, 본체 분해는 W5.2로 명시 분리 |

## 8. 산출물

- `PickedAtomInfo` 신규 정의
- render/measurement 경계 시그니처 변경본
- `measurement_vtk_token_inventory_phase19_latest.md`
- `dependency_gate_report_W2.md`

## 9. Git 커밋 템플릿

```
refactor(measurement,render): phase19/W2 — remove vtkActor* from measurement boundary

- Introduce PickedAtomInfo value object
- Route picker result through RenderGateway
- Change measurement click API to value-object contract
- Remove VTK headers from measurement/application

metrics:
  measurement vtk token refs: 1 -> 0

verif:
  check:phase15:render-platform-isolation: PASS
```

---

*W2 완료 시 Measurement 경계는 render 독립 계약으로 봉인된다.*
