# Phase 14 W6 메뉴 창 오픈 매트릭스

- 측정 시각: `2026-04-07 (KST)`
- 기준 코드: `webassembly/src/app.cpp` (`openEditorPanel/openBuilderPanel/openDataPanel`)
- 검증 방식:
  1. 정적 코드 경로 점검
  2. Phase 14 정적 게이트(`check_phase14_state_ownership_alias_zero.ps1`) 결과 반영

## 1) 메뉴별 점검 결과

| 메뉴 | 기대 동작 | 코드 경로 점검 | 결과 |
|---|---|---|---|
| `Edit > Atoms` | Created Atoms 창 오픈 + 포커스 | `openEditorPanel(Atoms)`에서 `m_bShowCreatedAtomsWindow = true`, `requestLocalFocus(CreatedAtoms)` | PASS |
| `Edit > Bonds` | Bonds Management 창 오픈 + 포커스 | `openEditorPanel(Bonds)`에서 `m_bShowBondsManagementWindow = true`, `requestLocalFocus(BondsManagement)` | PASS |
| `Edit > Cell` | Cell Information 창 오픈 + 포커스 | `openEditorPanel(Cell)`에서 `m_bShowCellInformationWindow = true`, `requestLocalFocus(CellInformation)` | PASS |
| `Build > Add atoms` | Periodic Table 창 오픈 + 포커스 | `openBuilderPanel(AddAtoms)`에서 `m_bShowPeriodicTableWindow = true`, `requestLocalFocus(PeriodicTable)` | PASS |
| `Build > Bravais Lattice Templates` | Crystal Templates 창 오픈 + 포커스 | `openBuilderPanel(BravaisLatticeTemplates)`에서 `m_bShowCrystalTemplatesWindow = true`, `requestLocalFocus(CrystalTemplates)` | PASS |
| `Data > Isosurface` | Charge Density Viewer 창 오픈 + 포커스 | `openDataPanel(Isosurface)`에서 `m_bShowChargeDensityViewerWindow = true`, `requestLocalFocus(ChargeDensityViewer)` | PASS |
| `Data > Surface` | Charge Density Viewer 창 오픈 + 포커스 | `openDataPanel(Surface)`에서 동일 경로 | PASS |
| `Data > Volumetric` | Charge Density Viewer 창 오픈 + 포커스 | `openDataPanel(Volumetric)`에서 동일 경로 | PASS |
| `Data > Plane` | Slice Viewer 창 오픈 + 포커스 | `openDataPanel(Plane)`에서 `m_bShowSliceViewerWindow = true`, `requestLocalFocus(SliceViewer)` | PASS |
| `Utilities > Brillouin Zone` | Brillouin Zone Plot 창 오픈 + 포커스 | `openBuilderPanel(BrillouinZone)`에서 `m_bShowBrillouinZonePlotWindow = true`, `requestLocalFocus(BrillouinZonePlot)` | PASS |

## 2) 동기화 가드 확인

1. 메뉴 처리 프레임 종료 시 `syncShellStateToStore()` 호출 유지.
2. 정적 게이트 항목 `P14.menu_open_regression_guard_codepath` PASS.

## 3) 결론

1. 메뉴 액션 코드 경로 기준 회귀 방지 조건은 충족(PASS).
2. 런타임 수동 시나리오(실제 클릭 기반 시각 확인)는 W6 외 수동 QA에서 추가 확인 권장.
