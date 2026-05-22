# 리팩토링 중간보고서 — Phase 19 (W0~W8)

- 작성일: `2026-05-22 (KST)`
- 최종 업데이트: `2026-05-22 (KST, W2 미해결 이슈 보완 반영)`
- 기준 계획서: `docs/refactoring/refactoring_plan_phase19_legacy_complete_dismantle_codex_260421_v2.md`
- 평가 범위: W0 ~ W8 수행 결과
- 근거 자료: `docs/refactoring/phase19/logs/dependency_gate_report_W0.md` ~ `dependency_gate_report_W8.md`, `phase19_inventory_snapshot_latest.md`

## 1. 종합 평가

- 진척도: **W0~W8 총 9개 Workstream 모두 실행 완료(형식상 100%)**
- 계획 대비 이행 수준: **높음**
- 중간 판정: **PASS**
- 판정 근거: 원계획 DoD(8번) 이슈였던 `measurement/application` VTK 타입 잔존을 해소했고, 보완 후 `build-wasm:release` 재검증까지 통과

## 2. WBS별 계획 대비 수행 현황 (W0~W8)

| WBS | 원계획 목표 | 수행 결과 | 판정 | 근거 |
|---|---|---|---|---|
| W0 | 기준선/인벤토리 동결 | 인벤토리/호출자맵/분해 매트릭스 동결 완료 | PASS | `dependency_gate_report_W0.md` |
| W1 | `friend class` 제거 | `friend class` 0 달성 | PASS | `dependency_gate_report_W1.md` |
| W2 | Measurement 경계 정리, `vtkActor*` 제거 | `measurement/application`의 VTK 타입 API 제거, render 계층으로 VTK 처리 이동 | PASS | `measurement_service.{h,cpp}`, `viewer_interaction_controller.cpp` |
| W3 | structure/measurement/density legacy port 제거 | `legacy_*_service_port` 참조 0 | PASS | `dependency_gate_report_W3.md` |
| W4 | IO/Render legacy 제거 | `io/infrastructure/legacy` 제거, `legacy_viewer_facade` 제거 | PASS | `dependency_gate_report_W4.md` |
| W5 | AtomsTemplate 완전 분해 | W5.1~W5.8 + 확장(W5.9, W5.10) 수행, `AtomsTemplate`/`LegacyAtomsRuntime`/`::Instance()` 0 | PASS (확장완료) | `dependency_gate_report_W5.md` |
| W6 | Singleton 제거 | W5.10 기준 반영 후 singleton-zero 게이트 유지, 테스트 PASS | PASS | `dependency_gate_report_W6.md` |
| W7 | `app.cpp` 재분해 (`<=400`) | `app.cpp` 73 LOC, 분해 파일군 생성 및 테스트 PASS | PASS | `dependency_gate_report_W7.md`, `app_cpp_decomposition_matrix_phase19_latest.md` |
| W8 | 물리 삭제/빌드 정리 | 남은 `legacy_atoms_runtime` 물리 삭제 및 빌드 그래프 정리 | PASS | `dependency_gate_report_W8.md` |

## 3. 정량 지표 비교 (원계획 기준선 대비 W8 시점)

| 항목 | 기준선 | W8 시점 | 평가 |
|---|---:|---:|---|
| legacy directories (`webassembly/src`) | 5 | 0 | 목표 달성 |
| `legacy` 파일명 파일 수 (`webassembly/src`) | 14 (폴더내 12 + 외부 2) | 0 | 목표 달성 |
| `friend class` | 1 | 0 | 목표 달성 |
| `AtomsTemplate` 심볼 | 다수(대형 facade 중심) | 0 | 목표 달성 |
| `LegacyAtomsRuntime` 심볼 | 다수 | 0 | 목표 달성 |
| `::Instance()` 호출 | 48 | 0 | 목표 달성 |
| `DECLARE_SINGLETON` 사용 | 4 | 1(매크로 정의만) | 목표 달성(실사용 0) |
| `app.cpp` LOC | 1,430 | 73 | 목표 초과 달성 |
| `measurement/application` VTK 토큰 | 1(기준선 보고) | 0(보완 후 재스캔) | 목표 달성 |

## 4. 원계획 대비 조정/편차 평가

| 구분 | 내용 | 영향 |
|---|---|---|
| 범위 확장 | W5에 W5.9(심볼 제로화), W5.10(`::Instance()` 전면 제거) 추가 수행 | 원계획 W6 리스크를 선제 흡수, 구조적 완성도 상승 |
| 순서 정합 | W6은 W5.10 이후 상태에 맞춰 계획서/DoD를 재정합 후 실행 | 중복 제거, 게이트 기준 명확화 |
| W8 범위 재정의 | 이미 정리된 legacy 항목 제외, 실제 잔존물(`legacy_atoms_runtime`)에 집중 | 실효성 높은 마무리 수행 |
| 편차 해소 | W2 목표(`measurement/application`에서 `vtkActor*` 0) 미충족 이슈를 후속 보완으로 해소 | Phase19 DoD 정합성 회복 |

## 5. 이슈 해소 내역 및 권고

- 해소된 이슈:
  - `webassembly/src/measurement/application/measurement_service.h`
  - `webassembly/src/measurement/application/measurement_service.cpp`
  - 조치: `vtkActor*`, `vtkRenderer*` 기반 API 제거
  - 대체 경로: VTK 의존 동작은 `webassembly/src/render/application/viewer_interaction_controller.cpp`에서 `WorkspaceRuntimeModelRef()` 경유로 처리
- 검증 결과:
  - `measurement/application` VTK 토큰 스캔 결과: `0`
  - `build-wasm:release`: `PASS`
- 권고:
  - W9/W10 종료 게이트에 `measurement/application` VTK 토큰 0 검증을 명시적으로 포함해 재유입을 차단
  - W2 관련 로그(`measurement_vtk_token_inventory_phase19_latest.md`)를 최신 실행 시각 기준으로 재기록 권장

## 6. 중간 결론

W0~W8은 원계획의 핵심 목표(legacy 물리 제거, 대형 legacy 책임 분해, singleton/Instance 제거, app.cpp 분해)를 높은 수준으로 달성했으며, 기존 미해결이던 Measurement 경계의 VTK 타입 잔존도 보완 완료했다. 따라서 Phase19 W8 기준 중간 평가는 **조건부 해제된 PASS 상태**로 본다.
