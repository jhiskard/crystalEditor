# Phase 19 / W5 — `AtomsTemplate` facade 완전 분해

작성일: `2026-04-21 (KST)`
상위 계획서: `docs/refactoring/refactoring_plan_phase19_legacy_full_dismantle_claude2_260421.md`
검증보고서 근거: 5.1 R1 (god-object 유지 5,808 LOC), 5.2 Low→ **완전 해체로 승격**
우선순위: **최고 작업량** (Phase 19 공수의 60% 이상)
예상 소요: 7 ~ 10일
선행 의존: W3 (포트 내재화), W4 (io/render legacy 제거)
후속 작업: W6 (singleton 제거), W7 (app.cpp 재분해)

---

## 1. 배경

`workspace/legacy/atoms_template_facade.{h,cpp}`(총 7,293 LOC)는 Phase 8에서 기존 `AtomsTemplate` 단일 거대 클래스를 **격리**하면서 만들어진 facade다. 이후 각 Phase에서 외부 호출 경로는 점진적으로 차단되었으나, facade 내부에는 여전히 다음 책임이 혼재한다:

- Atom/Bond/Cell 상호작용 (Select/Hover/Visibility/Toggle)
- Measurement 모드 전환 및 피킹 핸들링
- Charge Density 로딩/그리드/이소표면 제어
- Structure 등록/제거 및 Brillouin Zone 계산
- UI 창 렌더(`RenderPeriodicTableWindow`, `RenderChargeDensityViewerWindow`, ...)
- XSF/CHGCAR 파일 브릿지

W5는 이 186개(추정) public 메서드 전부를 **귀속처로 이관**하여 `AtomsTemplate` 클래스 자체를 소멸시킨다. 본 작업은 Phase 19 전체 공수의 대부분을 차지하며, 6개 서브 단계로 분리한다.

## 2. 사전 조건

- W0에서 `atoms_template_facade.cpp`의 186 메서드 전량을 "책임 귀속 매트릭스"로 분류 (`docs/refactoring/phase19/logs/atoms_template_ownership_matrix_phase19_latest.md`).
- W3/W4 완료로 외부 경로의 포트 레이어가 모두 내재화되어 있음.
- 각 feature의 도메인 레이어가 본 작업이 요구하는 Repository/Store 메서드를 노출하고 있음 (없으면 선 추가).

## 3. 책임 귀속 매트릭스 (개념)

| 책임 군 | 귀속 경로 | 예상 메서드 수 | 서브 단계 |
|---|---|---:|---|
| Atom/Bond/Cell 상호작용 | `structure/application/structure_interaction_service.{h,cpp}` (신규) | ~40 | W5.1 |
| Measurement 피킹/오버레이 | `measurement/application/measurement_service.{h,cpp}` 확장 + `measurement/presentation/measurement_overlay_renderer.{h,cpp}` (신규) | ~25 | W5.2 |
| Charge Density 로드/표시 | `density/application/density_service.{h,cpp}` 확장 + `density/presentation/charge_density_controller.{h,cpp}` (신규) | ~50 | W5.3 |
| Structure 라이프사이클/BZ | `structure/application/structure_lifecycle_service.{h,cpp}` 확장 | ~30 | W5.4 |
| UI 창 렌더 메서드 | `shell/presentation/atoms/*` (분산 신규) | ~25 | W5.5 |
| XSF/CHGCAR 브릿지 | `io/application/import_*_service.{h,cpp}` 확장 | ~16 | W5.6 |

## 4. 서브 단계 (완료 순서 고정)

### W5.1 — Atom/Bond/Cell Interaction → `structure_interaction_service`

#### 4.1 이관 대상 메서드 예

- `SelectAtom(int structureId, int atomId)`
- `HoverAtom(int structureId, int atomId)`
- `ToggleAtomVisibility(int structureId, int atomId, bool visible)`
- `SelectBond(int structureId, int bondId)`
- `ToggleCellVisibility(int structureId, bool visible)`
- 기타 ~40건

#### 4.2 신규 파일

- `webassembly/src/structure/application/structure_interaction_service.h`
- `webassembly/src/structure/application/structure_interaction_service.cpp`

#### 4.3 상태 귀속

| 기존 `AtomsTemplate` 멤버 | 목적지 |
|---|---|
| `m_SelectedAtoms` | `structure/domain/structure_selection_store` |
| `m_HoveredAtomId` | `structure/domain/structure_selection_store` |
| `m_AtomVisibility` | `structure/domain/structure_repository` (per-structure state) |

#### 4.4 절차

1. `StructureInteractionService` 클래스 골격 생성
2. 메서드 1건씩 이관 (커밋 쪼개기 권장: 10건/커밋)
3. 각 이관마다 `atoms_template_facade.cpp`에서 해당 메서드를 **얇은 delegation**으로 교체 (`AtomsTemplate::SelectAtom(...) { return m_StructureInteractionService.selectAtom(...); }`)
4. 외부 호출자를 인벤토리 후 `StructureInteractionService` 직접 호출로 전환
5. `AtomsTemplate`에서 해당 delegation 메서드 제거
6. 단위 테스트 `structure_interaction_service_test.cpp` 신규/확장

### W5.2 — Measurement 피킹/오버레이 → `measurement_service` + `measurement_overlay_renderer`

#### 4.5 전제

W2에서 `HandleMeasurementClickByPicker(PickedAtomInfo)` 서명이 이미 확정된 상태. W5.2는 이 함수 정의부를 `MeasurementService::handlePickerClick(PickedAtomInfo)`로 실제 이관한다.

#### 4.6 이관 대상 메서드 예

- `HandleMeasurementClickByPicker(PickedAtomInfo)` (~1 + 헬퍼)
- `SetMeasurementMode(MeasurementMode)` (W14 상태이관 이후 잔존분)
- `ClearMeasurementPicks()`
- overlay 렌더 메서드 ~10건 (예: `RenderMeasurementOverlay`)

#### 4.7 신규 파일

- `webassembly/src/measurement/presentation/measurement_overlay_renderer.{h,cpp}` — ImGui + VTK 2D overlay 전담

#### 4.8 상태 귀속

| 기존 멤버 | 목적지 |
|---|---|
| `m_MeasurementMode` | `measurement/domain/measurement_store` (이미 이관 완료 여부 확인) |
| `m_MeasurementPickedAtomIds` | `measurement/domain/measurement_store` |
| `m_MeasurementOverlay*` VTK 핸들 | `render/infrastructure/` 또는 `measurement/presentation/measurement_overlay_renderer` 내부 |

### W5.3 — Charge Density → `density_service` + `charge_density_controller`

#### 4.9 이관 대상 메서드 예

- `LoadChargeDensity(const std::string& path)`
- `SetIsoValue(double)`
- `ToggleVolumeRendering(bool)`
- `RenderChargeDensityViewerWindow()` (ImGui 창) → presentation으로
- 그리드/이소표면 VTK 파이프라인 관련 ~30건

#### 4.10 신규 파일

- `webassembly/src/density/presentation/charge_density_controller.{h,cpp}` — ImGui 창 포함 presentation 컨트롤러
- (선택) `density/infrastructure/charge_density_pipeline.{h,cpp}` — VTK 파이프라인 전담

#### 4.11 상태 귀속

| 기존 멤버 | 목적지 |
|---|---|
| `m_ChargeDensityIso*` | `density/domain/density_repository` |
| `m_ChargeDensityVolumeActor` | `density/infrastructure/` (VTK 캐시) |

### W5.4 — Structure 라이프사이클/BZ → `structure_lifecycle_service`

#### 4.12 이관 대상 메서드 예

- `AddStructure(Structure&&)`
- `RemoveStructure(int)`
- `ReplaceStructure(int, Structure&&)`
- `ComputeBrillouinZone(int structureId)`
- `AddSpecialPoint(int structureId, const SpecialPoint&)`
- 기타 ~30건

#### 4.13 파일

- `webassembly/src/structure/application/structure_lifecycle_service.{h,cpp}` (기존) 확장

### W5.5 — UI 창 렌더 메서드 → `shell/presentation/atoms/*`

#### 4.14 이관 대상 메서드 예

- `RenderPeriodicTableWindow()` → `shell/presentation/atoms/periodic_table_window.{h,cpp}`
- `RenderAtomInfoPanel()` → `shell/presentation/atoms/atom_info_panel.{h,cpp}`
- `RenderStructureListPanel()` → `shell/presentation/atoms/structure_list_panel.{h,cpp}`
- 기타 ~22건 → 개별 presenter 파일로 분산

#### 4.15 원칙

각 presenter는 **순수 ImGui 렌더 책임**만 가지며 상태는 생성자 주입된 `StructureRepository&`, `MeasurementStore&`, `DensityRepository&`, `ShellStateStore&`에서 읽는다. VTK 직접 조작 금지.

### W5.6 — XSF/CHGCAR 브릿지 → `io/application/import_*`

#### 4.16 이관 대상 메서드 예

- `HandleXSFFile(const std::string&)`
- `HandleXSFGridFile(const std::string&)`
- `HandleChgcarFile(const std::string&)`
- 파싱 후 Repository 등록 브릿지 ~16건

#### 4.17 파일

- `webassembly/src/io/application/import_xsf_service.{h,cpp}` (기존 확장)
- `webassembly/src/io/application/import_chgcar_service.{h,cpp}` (기존 확장)

### W5.7 — `workspace/legacy/` 파일 4종 물리 삭제

이관이 완료되어 `atoms_template_facade.{h,cpp}`에 남은 코드가 delegation 또는 공백이 된 시점에 다음을 수행:

1. `AtomsTemplate` 클래스의 남은 delegation 메서드 모두 제거
2. `LegacyAtomsRuntime()` 함수 마지막 호출자 제거 확인
3. 파일 삭제:
   - `webassembly/src/workspace/legacy/atoms_template_facade.h`
   - `webassembly/src/workspace/legacy/atoms_template_facade.cpp`
   - `webassembly/src/workspace/legacy/legacy_atoms_runtime.h`
   - `webassembly/src/workspace/legacy/legacy_atoms_runtime.cpp`
   - 디렉터리 `webassembly/src/workspace/legacy/` 제거

### W5.8 — CMake 모듈 갱신

- `webassembly/cmake/modules/wb_workspace.cmake`에서 legacy 엔트리 제거
- `wb_workspace` 타깃의 target_sources()에서 4개 파일 제거
- 의존성 검증: `npm run build-wasm:debug` PASS

## 5. 파일 변경 명세 (집계)

| 유형 | 개수 |
|---|---:|
| 신규 파일 (application + presentation) | ~12 |
| 확장 파일 (기존 service) | ~6 |
| 삭제 파일 (workspace/legacy) | 4 |
| 삭제 디렉터리 | 1 |
| CMake 수정 | 1 (`wb_workspace.cmake`) + 필요 시 feature별 추가 |
| 단위 테스트 신규/확장 | ~6 |

## 6. 완료 기준 (DoD)

| 항목 | 측정 | 기준 |
|---|---|---|
| `webassembly/src/workspace/legacy/` 존재 | find | 없음 |
| `AtomsTemplate` 심볼 전역 | grep | 0 (테스트, 주석 포함) |
| `LegacyAtomsRuntime` 심볼 전역 | grep | 0 |
| 새 서비스들의 단위 테스트 | ctest/jest | PASS |
| e2e 전 시나리오 | Playwright | PASS |
| 수동 UI 회귀 (Phase 18 체크리스트) | 수작업 | PASS |
| `npm run build:full` | build | PASS |

## 7. 리스크와 완화

| 리스크 | 영향 | 완화책 |
|---|---|---|
| 186개 메서드 이관 중 숨은 상호 의존 | 회귀 대량 발생 | 서브 단계별 독립 PR + 각 10건/커밋 쪼개기, 각 PR에 e2e 강제 실행 |
| 상태 이관 시 race condition 발견 | 멀티스레드(`-pthread`) 환경에서 데이터 레이스 | 상태를 Repository/Store로 이관 시 mutex 정책을 도메인 레이어가 명시 보유 |
| `ChargeDensity` VTK 파이프라인은 기존 설계가 `AtomsTemplate` 멤버 함수에 강결합 | W5.3가 예상보다 장기화 | 필요 시 `charge_density_pipeline` 내부에서 VTK 핸들만 유지하고 API는 값 교환으로 유지 |
| facade delegation 과도기 동안 게이트 일시 FAIL | CI 혼선 | 중간 커밋은 `[WIP]` 태그로 main 병합 없이 feature branch에서 진행 |
| 이관 중 `AtomsTemplate::OnTimer()` 등 드문 호출 누락 | 런타임 크래시 | W0에서 전체 메서드 목록 고정 + 매 서브 단계 종료 시 `grep AtomsTemplate webassembly/src \| wc -l`로 잔존 호출 수 추적 |

## 8. 산출물

- 신규 서비스/컨트롤러 12개 파일
- 기존 서비스 확장 6개 파일
- 단위 테스트 6종
- 삭제 파일 4개 + 폴더 1개
- 로그: `docs/refactoring/phase19/logs/atoms_template_decomposition_progress_W5_{1..8}.md` — 서브 단계별 진행
- `docs/refactoring/phase19/logs/dependency_gate_report_W5.md`

## 9. Git 전략

- 브랜치: `refactor/phase19-W5-atoms-template` (W5 전용 sub-branch)
- 각 서브 단계(W5.1~W5.8)별 feature branch fork
- 각 서브 단계 당 PR 1개, 메서드 10건/커밋 단위
- W5.7 (파일 삭제)은 W5.1~W5.6이 모두 병합된 이후 단독 PR

## 10. Git 커밋 템플릿 (서브 단계 예)

```
refactor(structure): phase19/W5.1 — move interaction methods from AtomsTemplate to StructureInteractionService (part 3/4)

- Migrate 10 methods: SelectAtom, HoverAtom, ToggleAtomVisibility, ...
- AtomsTemplate keeps thin delegation stubs for now (to be removed in W5.7)
- StructureInteractionService now owns selection state via StructureSelectionStore
- New unit tests cover select/hover/visibility flows

metrics:
  AtomsTemplate methods: 186 -> 156
  atoms_template_facade.cpp LOC: 5808 -> ~5200
  StructureInteractionService LOC: 0 -> 420
  LegacyAtomsRuntime() call sites: 14 -> 12

verif:
  structure unit tests: PASS
  e2e select/hover: PASS
```

---

*W5는 Phase 19의 **최대 위험 + 최대 가치** 작업이다. 완료 시 독립 검증보고서 5.1 R1의 5,808 LOC god-object가 0 LOC로 소멸한다.*
