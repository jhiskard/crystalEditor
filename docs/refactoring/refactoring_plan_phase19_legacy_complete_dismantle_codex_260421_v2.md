# 리팩토링 코드 수정계획서 — Phase 19 (독립평가 리스크 해소 + Legacy 완전 해체) v2

## 0. 문서 메타

- 작성일: 2026-04-21
- 문서 버전: v2
- 기준 문서:
  - `docs/refactoring/refactoring_verification_report_independent_260420.md` (특히 5장)
  - `docs/refactoring/refactoring_plan_phase19_legacy_full_dismantle_claude2_260421.md`
  - `docs/refactoring/phase19/W1~W8 세부계획서`
- 범위: `webassembly/src` 및 연관 CMake/검증 스크립트/결과 문서
- 목표 상태:
  - 외부 검증 보고서 5.1/5.2 리스크·권고 전량 이행
  - `webassembly/src` 하위 모든 `legacy` 폴더 완전 제거
  - legacy 책임을 현행 모듈 경계로 완전 편입

### 0.1 v2 반영 사항

1. 정량 기준선(파일/LOC/참조량) 고정
2. Port 인터페이스 유지/삭제 판단 기준 명문화
3. W5(AtomsTemplate 해체) 순차 실행 + Thin-shim 전략 명시
4. 메타 게이트(`check_phase19_final_closure.ps1`) 설계 추가
5. 로그/증빙 산출물 표준 세트 명시
6. Workstream 의존성 보정: `W7(app.cpp 재분해)`는 `W5` 이후 실행

---

## 1. 배경 및 문제정의

독립 평가 보고서(2026-04-20)는 아키텍처 전환 자체는 달성으로 보되, 다음을 잠재 리스크로 식별했다.

1. `workspace/legacy/atoms_template_facade.cpp` 대형 단일 파일
2. `render/application/legacy_viewer_facade.*` 잔존
3. `friend class` 1건 잔존
4. Phase 11 의도 대비 `app.cpp` 미분해
5. `measurement/application`의 VTK 토큰 잔존
6. `VtkViewer`/`MeshManager`/`FontManager` singleton 잔존

Phase 19 v2의 핵심은 "격리 유지"가 아니라 "실질 이관 + 물리 삭제"이다.

---

## 2. 현재 기준선 (2026-04-21 코드 스냅샷)

### 2.1 해체 대상 경로

- `legacy` 디렉터리: 5개
  1. `webassembly/src/workspace/legacy`
  2. `webassembly/src/structure/infrastructure/legacy`
  3. `webassembly/src/measurement/infrastructure/legacy`
  4. `webassembly/src/density/infrastructure/legacy`
  5. `webassembly/src/io/infrastructure/legacy`
- `legacy` 디렉터리 내부 파일: 12개
- 디렉터리 외 legacy 파일: 2개
  - `webassembly/src/render/application/legacy_viewer_facade.h`
  - `webassembly/src/render/application/legacy_viewer_facade.cpp`

### 2.2 파일/LOC 인벤토리

| 경로 | LOC |
|---|---:|
| `workspace/legacy/atoms_template_facade.h` | 1,485 |
| `workspace/legacy/atoms_template_facade.cpp` | 5,808 |
| `workspace/legacy/legacy_atoms_runtime.h` | 8 |
| `workspace/legacy/legacy_atoms_runtime.cpp` | 16 |
| `render/application/legacy_viewer_facade.h` | 16 |
| `render/application/legacy_viewer_facade.cpp` | 13 |
| `structure/infrastructure/legacy/legacy_structure_service_port.h` | 38 |
| `structure/infrastructure/legacy/legacy_structure_service_port.cpp` | 67 |
| `measurement/infrastructure/legacy/legacy_measurement_service_port.h` | 37 |
| `measurement/infrastructure/legacy/legacy_measurement_service_port.cpp` | 111 |
| `density/infrastructure/legacy/legacy_density_service_port.h` | 39 |
| `density/infrastructure/legacy/legacy_density_service_port.cpp` | 65 |
| `io/infrastructure/legacy/legacy_import_runtime_port.h` | 45 |
| `io/infrastructure/legacy/legacy_import_runtime_port.cpp` | 124 |
| **합계** | **7,872** |

### 2.3 참조량 기준선

| 항목 | 현재값 |
|---|---:|
| `LegacyAtomsRuntime()` 호출 파일 수 | 14 |
| `workspace/legacy/` 문자열 참조 파일 수 | 32 |
| `friend class` 선언 | 1 |
| `DECLARE_SINGLETON` 사용(정의 포함) | 4 |
| `::Instance()` 호출 | 48 |
| `app.cpp` LOC | 1,430 |

---

## 3. 독립평가 5장 대응 매핑

| 독립평가 항목 | 우선순위 | Phase 19 대응 Workstream | 완료 기준 |
|---|---|---|---|
| `friend class` 1건 | High | W1 | `friend class` 0 |
| `measurement_controller` VTK 토큰 | Medium | W2 | `measurement/application`의 `vtkActor*` 0 |
| feature 서비스가 legacy port 경유 | High | W3/W4 | `*_service_port`/`legacy_*_port` 정리 |
| `atoms_template_facade` 거대 파일 | High | W5 | `AtomsTemplate`/`LegacyAtomsRuntime` 심볼 0 |
| singleton 잔존 | Low | W6 | `DECLARE_SINGLETON` 사용 0, `::Instance()` 0 |
| `app.cpp` 과대 | Medium | W7 | `app.cpp <= 400`, 메뉴/레이아웃 파일 실생성 |
| legacy 폴더 전면 해체 | 최우선 | W8 | legacy 디렉터리/파일 0 |
| 봉인/재유입 차단 | High | W9/W10 | 메타 게이트 + 종료 문서 패키지 |

---

## 4. 설계 원칙 (v2 보강)

### 4.1 Port 인터페이스 유지/삭제 기준

Port(`*_service_port.h`, `import_runtime_port.h`)는 아래 기준으로 판단한다.

1. 유지: 테스트 더블 주입이나 복수 구현체 계획이 실제 존재할 때
2. 삭제: legacy 구현체가 유일하고 대체 구현 계획이 없을 때
3. 기본값: 삭제(YAGNI), 단 유지 시 근거를 `service_test_strategy_phase19_latest.md`에 기록

### 4.2 W5 해체 운영 규칙

1. `W5.1~W5.6`은 병렬 금지, 순차 실행
2. 각 서브 단계는 `Thin-shim` 허용
   - 1차: 신규 서비스로 이관
   - 2차: 호출자 전환
   - 3차: shim 제거
3. 각 서브 단계 종료 시 회귀 테스트 + 게이트 증빙 필수

### 4.3 조립 원칙

- `shell/runtime/workbench_runtime.cpp` 외부에서 legacy runtime/facade 조립 금지
- 상태 소유권은 Repository/Store 단일 소유 원칙 준수
- render 경계 외부에 `vtkActor*`, `vtkVolume*` 노출 금지

### 4.4 주석 작성 지침 승계 (Phase13+ 4.1 적용)

목적:

- 경계 전환 코드의 임시 shim/호환 경로를 추적 가능하게 남기고, 만료 시점에 제거를 강제한다.

핵심 규칙:

1. 신규/변경 public API는 Doxygen `@brief` 필수.
2. 경계 계약 또는 사용 제약이 있는 API는 `@details` 또는 `@note` 필수.
3. 임시 shim/bridge/compat 경로는 아래 3개 메타를 반드시 포함한다.
   - `@note Temporary compatibility path`
   - 제거 목표 단계 (예: `remove in Phase 19 W5`)
   - 추적 ID (예: `P19-W5-SHIM-01`)
4. `.h`는 계약(책임/입출력/소유권), `.cpp`는 이유(순서/제약/우회 배경) 중심으로 작성한다.
5. 동작이 바뀐 코드의 기존 주석 미갱신은 주석 누락과 동일하게 FAIL로 처리한다.
6. 만료된 임시 주석(목표 WBS를 초과한 항목)은 W10 종료 시점 0이어야 한다.

검증 방식:

- 기본 게이트: `check_phase18_doc_contract_full.ps1` 재활용
- Phase 19 WBS 종료 로그(`dependency_gate_report_W{N}.md`)에 아래 3항목을 매번 기록
  - 신규/변경 public API 주석 존재 여부
  - `Temporary` 주석의 제거 목표/추적 ID 존재 여부
  - 만료 임시 주석 0건 여부

### 4.5 Git 저장소 운영 지침 승계 (Phase13+ 4.2 적용)

브랜치 정책:

1. Phase 19 작업 브랜치는 `refactor/phase19-legacy-complete-dismantle`를 기본으로 사용한다.
2. WBS 단위 로컬 커밋을 필수로 하며, 커밋 메시지에 `phase19 WN`을 포함한다.
3. Phase 종료 후 회귀 수정은 `hotfix/phase19-*` 브랜치로 분리하고 종료 문서 사후 보완 섹션과 연결한다.

동기화 정책:

1. 최소 동기화 단위는 W 종료 시점이며, W5는 서브 단계(W5.1~W5.6) 단위 동기화를 권장한다.
2. 원격 공유 이후 WBS 추적성을 훼손하는 rebase/squash 이력 재작성은 금지한다.
3. force push는 긴급 승인 케이스 외 금지한다.

산출물 추적 정책:

1. Phase 19 산출물은 `docs/refactoring/phase19/` 및 `docs/refactoring/` 지정 경로에만 저장한다.
2. `docs/refactoring/phase19/logs/*`는 `.gitignore` 누락 없이 추적 상태를 유지한다.
3. 실행 로그는 명령/시각/결과를 함께 남긴다.
4. 각 W 종료 시 `dependency_gate_report_W{N}.md` 갱신을 필수로 한다.

---

## 5. 실행 Workstream (W0~W10)

### W0. 기준선 고정/인벤토리

- legacy 경로, 참조량, 호출자 맵, CMake 소스 맵 동결
- 산출물:
  - `docs/refactoring/phase19/logs/phase19_inventory_snapshot_latest.md`
  - `docs/refactoring/phase19/logs/legacy_runtime_caller_inventory_phase19_latest.md`
  - `docs/refactoring/phase19/logs/atoms_template_ownership_matrix_phase19_latest.md`
  - `docs/refactoring/phase19/logs/app_cpp_decomposition_matrix_phase19_latest.md`

### W1. `friend class` 제거 (High)

- 대상: `io/application/import_entry_service.h:76`
- `ImportPopupState` 값 객체 또는 동등 API 계약으로 private 접근 제거
- 신규 게이트: `check_phase19_friend_class_zero.ps1`

### W2. Measurement 경계 정리 (Medium)

- `PickedAtomInfo` 값 객체 도입
- `RenderGateway::ResolvePickedAtom(...)` 경계 추가
- `measurement/application`에서 `vtkActor*` 제거
- 기존 게이트: `check_phase15_render_platform_isolation.ps1` 재검증

### W3. Structure/Measurement/Density 포트 내재화

- `legacy_*_service_port.{h,cpp}` 제거
- 서비스가 Repository/Store 직접 주입 구조로 전환
- Port 유지/삭제는 4.1 기준으로 판정 및 기록

### W4. IO/Render legacy 제거

- `io/infrastructure/legacy/legacy_import_runtime_port.*` 제거
- `render/application/legacy_viewer_facade.*` 제거
- `WorkbenchRuntime::viewer()`/직접 주입 경로로 치환

### W5. `AtomsTemplate` 완전 분해 (최대 작업량)

- W5.1 Atom/Bond/Cell
- W5.2 Measurement
- W5.3 Density
- W5.4 Structure lifecycle/BZ
- W5.5 UI Render Window
- W5.6 XSF/CHGCAR bridge
- W5.7 `workspace/legacy` 파일 4종 삭제
- W5.8 `wb_workspace.cmake` 정리

운영 규칙:

- `W5.1~W5.6` 순차 실행
- 각 단계별 개별 PR/회귀/게이트 증빙
- 단계 종료 시점마다 `atoms_template_call_graph_phase19_latest.md` 갱신

### W6. Singleton 제거 (Low)

- `VtkViewer`, `MeshManager`, `FontManager`의 `DECLARE_SINGLETON` 제거
- `::Instance()` 호출 0화
- `macro/singleton_macro.h`는 삭제 또는 deprecated 봉인(사용 0 강제)

### W7. `app.cpp` 재분해 (Medium, W5 이후)

- 신규 파일:
  - `shell/presentation/main_menu.{h,cpp}`
  - `shell/presentation/window_layout.{h,cpp}`
  - `shell/presentation/window_registry.{h,cpp}`
  - `shell/presentation/popup_presenter.{h,cpp}`
  - `shell/presentation/font/font_scale_controller.{h,cpp}`
  - `shell/presentation/theme/color_style_controller.{h,cpp}`
- 목표: `app.cpp <= 400 LOC`

### W8. 물리 삭제/빌드 정리

- 남은 legacy 경로 전수 삭제
- `webassembly/cmake/modules/wb_*.cmake`에서 legacy 엔트리 0
- `find legacy dir == 0`, `legacy token == 0` 재확인

### W9. Phase19 게이트 패키지/메타게이트 구성

- 신규 게이트:
  - `check_phase19_legacy_directory_zero.ps1`
  - `check_phase19_atoms_template_zero.ps1`
  - `check_phase19_legacy_runtime_zero.ps1`
  - `check_phase19_singleton_zero.ps1`
  - `check_phase19_friend_class_zero.ps1`
  - `check_phase19_app_cpp_size.ps1`
  - `check_phase19_final_closure.ps1`
- `package.json`에 `check:phase19:*` 추가

### W10. 종료 검증/결과보고/봉인

- `build-wasm:release`, `test:cpp`, `test:smoke`, `check:phase19:final` PASS
- 주석 계약 점검(`check_phase18_doc_contract_full.ps1` + WBS 로그 점검 항목) PASS
- 저장소 운영 점검(`phase19 WN` 커밋 추적성, `dependency_gate_report_W{N}.md` 완결성) PASS
- 결과 문서:
  - `docs/refactoring/refactoring_result_report_phase19_legacy_complete_dismantle_260421.md`
  - `docs/refactoring/phase19/go_no_go_phase19_close.md`
  - `docs/refactoring/phase19/logs/*`

---

## 6. 메타 게이트 설계 (v2 추가)

`check_phase19_final_closure.ps1`는 아래 순서로 실행한다.

### 6.1 Phase 19 신규 게이트

1. `check_phase19_legacy_directory_zero.ps1`
2. `check_phase19_atoms_template_zero.ps1`
3. `check_phase19_legacy_runtime_zero.ps1`
4. `check_phase19_singleton_zero.ps1`
5. `check_phase19_friend_class_zero.ps1`
6. `check_phase19_app_cpp_size.ps1`

### 6.2 기존 게이트 상속 (스크립트명 정합 반영)

1. `check_phase7_runtime_composition.ps1`
2. `check_phase12_architecture_seal.ps1`
3. `check_phase13_runtime_hard_seal.ps1`
4. `check_phase14_state_ownership_alias_zero.ps1`
5. `check_phase15_render_platform_isolation.ps1`
6. `check_phase17_root_allowlist.ps1`
7. `check_phase18_facade_include_zero.ps1`
8. `check_phase18_atoms_instance_zero.ps1`
9. `check_phase18_legacy_singleton_zero.ps1`
10. `check_phase18_doc_contract_full.ps1`
11. `check_phase18_final_closure.ps1`

---

## 7. 로그/증빙 표준 산출물 (v2 추가)

### 7.1 공통 로그

- `docs/refactoring/phase19/logs/phase19_inventory_snapshot_latest.md`
- `docs/refactoring/phase19/logs/legacy_runtime_caller_inventory_phase19_latest.md`
- `docs/refactoring/phase19/logs/atoms_template_ownership_matrix_phase19_latest.md`
- `docs/refactoring/phase19/logs/atoms_template_call_graph_phase19_latest.md`
- `docs/refactoring/phase19/logs/w5_state_ownership_matrix_phase19_latest.md`
- `docs/refactoring/phase19/logs/app_cpp_decomposition_matrix_phase19_latest.md`
- `docs/refactoring/phase19/logs/service_test_strategy_phase19_latest.md`
- `docs/refactoring/phase19/logs/dependency_gate_report_W{1..10}.md`
- `docs/refactoring/phase19/logs/manual_ui_regression_phase19_latest.md`

### 7.2 종료 문서

- `docs/refactoring/refactoring_result_report_phase19_legacy_complete_dismantle_260421.md`
- `docs/refactoring/phase19/go_no_go_phase19_close.md`
- 필요 시: `docs/refactoring/refactoring_result_report_phase0_19_comprehensive_260421.md`

---

## 8. Definition of Done (Phase 19 종료 조건)

1. 독립평가 5.2 권고 1~5 전량 이행
2. `webassembly/src` 하위 legacy 디렉터리 5개 전부 삭제
3. legacy 코드 파일 12개 + `legacy_viewer_facade` 2개 삭제
4. `AtomsTemplate`, `LegacyAtomsRuntime`, `legacy_*_service_port`, `legacy_viewer_facade` 심볼 0
5. `friend class` 0
6. `DECLARE_SINGLETON` 사용 0, `::Instance()` 0
7. `app.cpp <= 400 LOC`, `main_menu.cpp`/`window_layout.cpp` 실존
8. `measurement/application` 내 `vtkActor*` 0
9. `build-wasm:release`, `test:cpp`, `test:smoke`, `check:phase19:final` PASS
10. 종료 문서/로그 패키지 완비
11. 주석 계약 준수:
   - 신규/변경 public API Doxygen 주석 존재
   - `Temporary compatibility path` 주석의 제거 목표/추적 ID 존재
   - 만료 임시 주석 0
12. 저장소 운영 준수:
   - WBS 단위 커밋 추적 가능 (`phase19 WN` 표기)
   - `dependency_gate_report_W{N}.md` 전 단계 갱신
   - 원격 공유 이후 이력 재작성 금지 원칙 준수

---

## 9. 리스크 및 완화

| 리스크 | 영향 | 완화 |
|---|---|---|
| W5 대형 분해 중 회귀 | 기능 붕괴/무한 루프 | 순차 분해 + Thin-shim + 단계별 회귀 |
| Port 삭제로 테스트성 저하 | 단위 테스트 취약 | 4.1 기준으로 유지/삭제 판정 + 전략 문서화 |
| `app.cpp` 분해 중 레이아웃 회귀 | UX 회귀 | `workbench-layout-panel-regression` + 수동 체크리스트 |
| Singleton 제거 시 초기화 순서 이슈 | 부팅 실패 | `WorkbenchRuntime` 조립 순서 문서화/검증 |
| 게이트 이름 불일치 | 자동화 실패 | 실제 스크립트명(underscore) 기준 표준화 |

---

## 10. 실행 순서 요약

1. W0~W2: 기준선 고정 + 고위험 경계(friend/measurement) 선해결
2. W3~W4: 포트/어댑터/legacy facade 제거
3. W5: AtomsTemplate 완전 분해(순차)
4. W6~W7: singleton 제거 + app.cpp 재분해
5. W8~W10: 물리 삭제 최종 점검 + 메타게이트 + 종료 보고

Phase 19 v2의 최종 판정 기준은 하나다: **legacy 격리 상태를 남기지 않고, 기능 이관과 코드/폴더 삭제를 동시에 완료하는 것**.
