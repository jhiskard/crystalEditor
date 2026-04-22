# 리팩토링 코드 수정계획서 — Phase 19 (독립평가 리스크 해소 + Legacy 완전 해체)

## 0. 문서 메타

- 작성일: 2026-04-21
- 기준 문서:
  - `docs/refactoring/refactoring_verification_report_independent_260420.md`
  - 특히 5장 `리스크 및 후속 과제`(5.1, 5.2)
- 범위: `webassembly/src` 및 연관 CMake/게이트/검증 문서
- 목표 상태:
  - 외부 검증 보고서 권고사항 이행
  - `webassembly/src` 하위 모든 `legacy` 폴더 및 코드 완전 해체
  - 해체 기능을 현행 모듈 경계(`shell/render/structure/measurement/density/io/workspace`)로 완전 편입

---

## 1. 배경 및 문제정의

독립 평가 보고서(2026-04-20)는 아키텍처 전환 자체는 달성으로 보되, 다음 항목을 잠재 리스크로 지목했다.

1. `workspace/legacy/atoms_template_facade.cpp` 대형 단일 파일(5,808 lines)
2. `render/application/legacy_viewer_facade.*` 잔존
3. `friend class` 1건(`io/application/import_entry_service.h`)
4. Phase 11 의도 대비 `app.cpp` 미분해(1,430 lines)
5. `measurement/application` 내부 `vtkActor*` 토큰 잔존
6. 장기과제: `VtkViewer`, `MeshManager`, `FontManager` singleton 제거

본 Phase 19는 위 항목을 단순 문서화가 아닌 **코드 변경 중심**으로 해소하며, 특히 legacy 격리전략을 종료하고 **완전 해체/편입**으로 전환한다.

---

## 2. 해체 대상(확정 인벤토리)

### 2.1 Legacy 폴더 (총 5개)

1. `webassembly/src/workspace/legacy`
2. `webassembly/src/structure/infrastructure/legacy`
3. `webassembly/src/measurement/infrastructure/legacy`
4. `webassembly/src/density/infrastructure/legacy`
5. `webassembly/src/io/infrastructure/legacy`

### 2.2 Legacy 코드 파일 (총 12개)

1. `workspace/legacy/atoms_template_facade.h`
2. `workspace/legacy/atoms_template_facade.cpp`
3. `workspace/legacy/legacy_atoms_runtime.h`
4. `workspace/legacy/legacy_atoms_runtime.cpp`
5. `structure/infrastructure/legacy/legacy_structure_service_port.h`
6. `structure/infrastructure/legacy/legacy_structure_service_port.cpp`
7. `measurement/infrastructure/legacy/legacy_measurement_service_port.h`
8. `measurement/infrastructure/legacy/legacy_measurement_service_port.cpp`
9. `density/infrastructure/legacy/legacy_density_service_port.h`
10. `density/infrastructure/legacy/legacy_density_service_port.cpp`
11. `io/infrastructure/legacy/legacy_import_runtime_port.h`
12. `io/infrastructure/legacy/legacy_import_runtime_port.cpp`

---

## 3. 보고서 5장 항목별 이행 매핑

| 독립평가 5장 항목 | 우선순위 | 코드 수정 이행안 | 완료 기준 |
|---|---|---|---|
| `friend class` 1건 | High | `FileLoader`에 Presenter 전용 public API(POD 상태 스냅샷 + confirm/cancel/dismiss) 추가 후 `friend class` 제거 | `rg "friend\\s+class" webassembly/src` 결과 0 |
| `app.cpp` 재분해 | Medium | `shell/presentation/main_menu.cpp`, `shell/presentation/window_layout.cpp` 실생성 및 메뉴/레이아웃 블록 이관 | `app.cpp` ≤ 400 lines, `App::` 메서드 축소 |
| measurement VTK 토큰 제거 | Medium | `AtomsTemplate::HandleMeasurementClickByPicker` 경로를 render picking + measurement target DTO로 분리 | `measurement/application` 내 `vtkActor` 토큰 0 |
| `atoms_template_facade` 축소 | Low(실행은 핵심) | S1~S8 슬라이스 단위 기능 이관 후 클래스/파일 삭제 | `AtomsTemplate` 심볼 0 |
| singleton 장기 제거 계획 | Low | 본 Phase에서 호출 지점 축소 + DI 경로 확정, 제거는 W8에서 가능한 범위 반영 | `DECLARE_SINGLETON`/`::Instance()` 잔여량 명시·감축 |
| `legacy_viewer_facade` 잔존 | Medium | `RenderScenePort`/`VtkRenderGateway` 경로로 치환 후 파일 삭제 | `legacy_viewer_facade` 토큰 0 |
| Legacy 폴더 전면 해체(본 요청) | 최우선 | 5개 폴더/12개 파일 기능 이관 후 물리 삭제 + CMake 정리 | `find legacy dir == 0`, `rg legacy token == 0` |

---

## 4. 기능 이관 목표 맵 (Legacy → 현재 모듈)

### 4.1 `workspace/legacy` 계열

- `AtomsTemplate` 구조/원자/셀 관련: `structure/application/*`, `structure/domain/*`, `render/infrastructure/atoms/*`
- 측정 관련: `measurement/application/*`, `measurement/domain/*`, `measurement/infrastructure/*`
- 밀도/슬라이스 관련: `density/application/*`, `density/domain/*`, `density/infrastructure/*`
- 파일 입력(XSF/CHGCAR) 관련: `io/application/*`, `io/infrastructure/*`
- 선택/호버/툴팁: `render/application/*`, `render/infrastructure/interaction/*`, `shell/presentation/atoms/*`
- 레이아웃/패널 UI: `shell/presentation/*`

### 4.2 4개 Legacy Port Adapter

- `legacy_structure_service_port` 기능: `structure/application/structure_service` + `structure/application/structure_lifecycle_service`
- `legacy_measurement_service_port` 기능: `measurement/application/measurement_service`
- `legacy_density_service_port` 기능: `density/application/density_service`
- `legacy_import_runtime_port` 기능: `io/application/import_entry_service` + `io/application/import_workflow_service` + `io/application/import_apply_service`

### 4.3 런타임 조립 원칙

- `shell/runtime/workbench_runtime.cpp`에서 legacy 포트/런타임 진입점 조립 금지
- 신규/기존 feature service를 Composition Root에서 직접 조립
- feature 간 호출은 포트가 필요한 경우를 제외하고 service/repository 경계로 직접 연결

---

## 5. 실행 Workstream (W0~W10)

### W0. 기준선 고정 및 영향도 분석

- `legacy` 경로 참조 인벤토리(`include`, 심볼, 생성자 주입, CMake 소스 목록) 생성
- `AtomsTemplate` 메서드 인벤토리 작성(메서드 → 타겟 모듈 매핑 CSV)

산출물:

- `docs/refactoring/phase19/logs/legacy_inventory_phase19_latest.md`
- `docs/refactoring/phase19/logs/atoms_template_method_map_phase19_latest.csv`

### W1. `friend class` 제거 (High)

- 대상: `io/application/import_entry_service.h`
- `ImportPopupPresenter`가 쓰는 private 접근점을 public API로 대체
- `friend class shell::presentation::ImportPopupPresenter;` 제거

### W2. `app.cpp` 실분해 (Medium)

- `shell/presentation/main_menu.cpp` 신설
- `shell/presentation/window_layout.cpp` 신설
- `renderDockSpaceAndMenu` 및 레이아웃 관련 로직 이관

### W3. Measurement 경계 정리 (Medium)

- `measurement/application`에서 VTK 타입 의존 제거
- 클릭 입력은 render 계층에서 도메인 DTO로 변환 후 measurement service 전달

### W4. Legacy Port 병행 치환

- 비-legacy 어댑터/서비스 경로 도입
- `workbench_runtime` 조립에서 legacy 포트 사용 중단

### W5. `AtomsTemplate` 기능 슬라이스 이관 I

- 구조/셀/원자 표시, 본드, 선택/호버 파트를 우선 이관
- 이관 즉시 legacy 구현 본문 제거(호출 경로 치환 완료 후)

### W6. `AtomsTemplate` 기능 슬라이스 이관 II

- 측정/밀도/파일입력/UI 창 렌더 파트 이관
- `LegacyAtomsRuntime()` 호출 사이트 전면 제거

### W7. `legacy_viewer_facade` 제거

- `render/application/legacy_viewer_facade.{h,cpp}` 삭제
- 호출자를 `RenderScenePort`/`VtkRenderGateway`로 전환

### W8. Singleton 감축/봉인

- `VtkViewer`, `MeshManager`, `FontManager` 호출지 DI 전환 우선 적용
- 남는 항목은 allowlist + 후속 제거 로드맵 문서화

### W9. 물리 삭제 및 빌드 시스템 정리 (핵심)

- 5개 `legacy` 폴더 및 12개 파일 삭제
- `webassembly/cmake/modules/wb_*.cmake`에서 legacy 소스 엔트리 제거

### W10. 최종 검증/문서화

- 자동 게이트 + 단위 테스트 + 스모크 + 수동 UI 회귀 수행
- 결과보고서/Go-NoGo 문서 발행

---

## 6. 검증 게이트 (신규/강화)

1. `check:phase19:friend-class-zero`
2. `check:phase19:app-cpp-split`
3. `check:phase19:measurement-vtk-zero`
4. `check:phase19:legacy-runtime-zero`
5. `check:phase19:legacy-viewer-zero`
6. `check:phase19:ports-dissolved`
7. `check:phase19:legacy-dirs-zero`
8. `check:phase19:legacy-token-zero`
9. `check:phase19:atoms-template-class-zero`
10. `check:phase19:all`

게이트 기준 예시:

- `find webassembly/src -type d -name legacy` 결과 0
- `rg "legacy_atoms_runtime|atoms_template_facade|legacy_.*_service_port|legacy_viewer_facade" webassembly/src` 결과 0
- `rg "ServicePort\\b|ImportRuntimePort\\b" webassembly/src` 결과 0 (계획한 dissolve 이후)

---

## 7. Definition of Done (Phase 19 종료 조건)

1. 독립평가 보고서 5.2 권고 1~5 이행 완료 또는 승인된 대체안 문서화
2. `webassembly/src` 하위 `legacy` 폴더 5개 전부 삭제
3. Legacy 코드 파일 12개 전부 삭제
4. `AtomsTemplate`, `LegacyAtomsRuntime`, `legacy_*_service_port`, `legacy_viewer_facade` 심볼 0
5. `app.cpp` 분해 완료(신설 파일 2개 이상 + 라인수/메서드수 기준 충족)
6. `measurement/application`에서 `vtkActor` 포함 VTK 타입 참조 0
7. `build-wasm:release`, `test:cpp`, `test:smoke`, `check:phase19:all` PASS
8. 결과 문서:
   - `docs/refactoring/refactoring_result_report_phase19_legacy_complete_dismantle_260421.md`
   - `docs/refactoring/phase19/go_no_go_phase19_close.md`
   - `docs/refactoring/phase19/logs/*`

---

## 8. 리스크 및 완화

- 대규모 이관 중 동작 회귀 위험
  - 완화: 슬라이스 단위 PR, 각 슬라이스 종료 시 스모크/수동 체크
- 런타임 조립 순서 변경에 따른 초기화 오류
  - 완화: `workbench_runtime` 조립 순서 문서화 + 회귀 테스트
- Port dissolve 과정의 의존성 순환
  - 완화: 서비스 생성 순서 고정 및 인터페이스 축소

---

## 9. 실행 순서 요약

1. W0~W1: 분석/고위험(`friend`) 선해결
2. W2~W4: 경계 정리(`app.cpp`, measurement, runtime 조립)
3. W5~W7: legacy 기능 실이관
4. W8~W9: singleton 감축, legacy 물리 삭제
5. W10: 게이트 통합 검증 및 종료 보고

본 계획의 핵심 판정 기준은 단일하다: **legacy 격리 상태를 유지하는 것이 아니라, 코드와 폴더를 완전히 제거하고 모듈 체계에 실질 편입하는 것**.
