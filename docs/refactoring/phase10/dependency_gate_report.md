# Phase 10 Workspace/Mesh/IO Decouple 게이트 리포트

작성일: `2026-04-03 (KST)`  
기준 계획: `docs/refactoring/phase10/refactoring_phase10_workspace_mesh_io_decouple_260402.md`  
상위 계획: `docs/refactoring/refactoring_plan_full_modular_architecture_260331.md` (Phase 10 절)  
작업 브랜치: `refactor/phase10-workspace-mesh-io-decouple`

근거 로그:
- 정적 게이트: `docs/refactoring/phase10/logs/check_phase10_workspace_mesh_io_decouple_latest.txt`
- WASM 빌드: `docs/refactoring/phase10/logs/build_phase10_latest.txt`
- C++ 테스트: `docs/refactoring/phase10/logs/unit_test_phase10_latest.txt`
- 브라우저 smoke: `docs/refactoring/phase10/logs/smoke_phase10_latest.txt`
- 인벤토리: `docs/refactoring/phase10/logs/workspace_mesh_io_inventory_phase10_latest.md`
- 버그 추적: `docs/refactoring/phase10/logs/bug_p10_vasp_grid_sequence_latest.md`

## 1. 필수 정적 게이트 결과
`check_phase10_workspace_mesh_io_decouple.ps1` 실행 결과 전 항목 PASS.

핵심 PASS 항목:
1. `workspace`/`mesh`/`import_workflow` 계약 파일 존재
2. `FileLoader`가 `ImportWorkflowService`를 사용
3. `import_orchestrator`/`import_workflow` 경로의 `MeshManager::Instance()` 직접 호출 0건
4. panel 경로(`model_tree`, `model_tree_*`, `mesh_group_detail`)의 `MeshManager::Instance()`/`FileLoader::Instance()` 직접 호출 0건
5. Phase 9 보호 규칙 유지
   - `render` 외부 `VtkViewer::Instance()` 호출 0건
   - `PrimeLegacySingletons`의 `FontRegistry` 미포함
   - `main.cpp`의 ImGui context 이후 폰트 초기화 순서 유지
   - `ChargeDensityUI` first-grid bootstrap guard 유지
6. 버그 로그 존재 + `P9-BUG-01` + 상태 태그 존재

## 2. 실행 게이트 결과
1. `npm run build-wasm:release`
   - 판정: `PASS`
   - 근거: `build_phase10_latest.txt`에서 configure/generate/install 완료, wasm 산출물 up-to-date 확인

2. `npm run test:cpp`
   - 판정: `PASS`
   - 근거: `unit_test_phase10_latest.txt`에서 `1/1` 테스트 PASS

3. `npm run test:smoke`
   - 판정: `PASS`
   - 근거: `smoke_phase10_latest.txt`에서 `1 passed`

## 3. 로그 해석 주의사항
`build_phase10_latest.txt`, `unit_test_phase10_latest.txt`에는 PowerShell 파이프라인에서 `emsdk_env.bat` 출력을 `NativeCommandError` 레코드로 표기하는 라인이 포함되어 있다.  
해당 라인은 게이트 실패가 아니라 stderr 라우팅 부산물이며, 같은 로그 내 빌드/테스트 본문 결과는 모두 PASS로 확인된다.

## 4. P9-BUG-01 판정
- 이슈 ID: `P9-BUG-01`
- 상태: `Deferred`
- 근거:
  1. W3에서 `ImportWorkflowService`를 도입해 import transaction 경계를 `FileLoader`에서 분리했다.
  2. W4에서 workspace selected mesh 문맥 동기화를 반영했다.
  3. 다만 반복 시퀀스(`XSF(Grid) -> VASP -> XSF(Grid) -> VASP`) 전용 자동 회귀는 아직 미구축이다.
- 이관:
  - Phase 11에서 전용 시나리오 자동화 또는 수동 검증 체크리스트 강화를 통해 최종 `Resolved/Deferred` 확정.

## 5. 잔여 리스크
| ID | 리스크 | 수준 | 대응 |
|---|---|---|---|
| P10-R1 | `mesh_detail.cpp` 잔여 `MeshManager::Instance()` 호출(32건) | Medium | Phase 11 panel 객체화 단계에서 service 경유로 추가 축소 |
| P10-R2 | `FileLoader::Instance()` 정적 entry(3건) 유지 | Medium | Phase 11~12에서 runtime facade 경유 또는 완전 교체로 0건화 |
| P10-R3 | `P9-BUG-01` 반복 시퀀스 전용 자동 회귀 부재 | Medium | Phase 11 smoke 확장 또는 전용 regression script 도입 |

## 6. 요약
- W6 기준 정적/동적 게이트는 모두 PASS다.
- Phase 10의 W0~W6 패키지(코드/로그/문서)가 정리되었고, Phase 11 착수 판단에 필요한 증적이 준비되었다.
- `P9-BUG-01`은 구조 분리 진척 후에도 `Deferred`로 유지되며 차기 Phase 우선 추적 항목이다.
