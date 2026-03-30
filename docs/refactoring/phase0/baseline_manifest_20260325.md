# Phase 0 Baseline Manifest (2026-03-27)

## 1. 메타 정보
- 기준 계획 문서:
  - `refactoring_plan_260324.md`
  - `refactoring_phase0_stabilization_baseline_260325.md`
- 측정 일시(KST): `2026-03-27 10:52:30 +09:00`
- 측정자: `user` + Codex
- 브랜치: `main`
- 기준 커밋:
  - Full: `763ad6417a831fad0b4840d470541ab4e39c841d`
  - Short: `763ad64`
- 워킹트리 상태(측정 시점): `M package.json`, `M package-lock.json` (사전 변경분)
- 측정 루트: `c:\Users\user\Downloads\vtk-workbench_jclee`

## 2. 기준선 재측정 커맨드
```powershell
git rev-parse --abbrev-ref HEAD
git rev-parse HEAD
rg --files webassembly/src | Measure-Object
rg -n "AtomsTemplate::Instance\(" webassembly/src | Measure-Object
rg -n "VtkViewer::Instance\(" webassembly/src | Measure-Object
rg -n "MeshManager::Instance\(" webassembly/src | Measure-Object
rg -n '#include ".*\.\./(app|vtk_viewer|model_tree|mesh_detail|mesh_manager)\.h"|#include ".*\.\./\.\./(app|vtk_viewer|model_tree|mesh_detail|mesh_manager)\.h"' webassembly/src/atoms
rg --files webassembly/src | rg "refactory.*\.md"
```

## 3. 정량 기준선 스냅샷
| 항목 | 값 | 비고 |
|---|---:|---|
| `webassembly/src` 파일 수 | 130 | 계획서 기준과 동일 |
| `AtomsTemplate::Instance()` 호출 수 | 72 | 전체 파일 기준 |
| `VtkViewer::Instance()` 호출 수 | 216 | 전체 파일 기준 |
| `MeshManager::Instance()` 호출 수 | 84 | 보강 지표 |
| `atoms` 상위 include 위반 수 | 15 | 패턴 기준 |
| 레거시 `refactory*.md` 파일 수 | 7 | `webassembly/src` 내부 |

### 대형 파일 라인 수(현재 측정)
| 파일 | Lines |
|---|---:|
| `webassembly/src/atoms/atoms_template.cpp` | 6131 |
| `webassembly/src/vtk_viewer.cpp` | 2449 |
| `webassembly/src/model_tree.cpp` | 1783 |
| `webassembly/src/file_loader.cpp` | 1282 |

### 코드 파일(`*.cpp`, `*.h`)만 집계한 singleton 호출 수
| 항목 | 값 |
|---|---:|
| `AtomsTemplate::Instance()` | 67 |
| `VtkViewer::Instance()` | 199 |
| `MeshManager::Instance()` | 84 |

## 4. 빌드 기준선 로그

### 4.1 이번 실행 시도 로그
| 시도 | 명령 | 결과 | 로그 |
|---|---|---|---|
| 1 | `npm.cmd run build-wasm:release` | 실패 (`emcmake` 미설치) | `docs/refactoring/phase0/logs/build_20260327_1730.txt` |
| 2 | `cmake --build webassembly/build --target install` | 실패 (기존 `CMakeCache.txt` 경로 불일치) | `docs/refactoring/phase0/logs/build_20260327_1734_cmake_build.txt` |
| 3 | Docker 기반 재빌드 | 실패 (Docker daemon 미기동) | `docs/refactoring/phase0/logs/build_20260327_1740_docker.txt` |
| 4 | `emsdk_env` 로드 후 `npm.cmd run build-wasm:release` | 성공 | `docs/refactoring/phase0/logs/build_20260327_1645_emsdk_success_attempt.txt` |

### 4.2 기존 성공 빌드 증거(히스토리)
- `webassembly/build/.ninja_log`에 `VTK-Workbench.js`/오브젝트 빌드 이력 존재
- 산출물 존재 확인:
  - `webassembly/build/VTK-Workbench.wasm` (2026-03-24 17:55:50)
  - `webassembly/build/VTK-Workbench.js` (2026-03-24 17:55:50)
  - `webassembly/build/VTK-Workbench.data` (2026-03-24 17:52:38)
  - `public/wasm/VTK-Workbench.{wasm,js,data}` 동일 타임스탬프/크기

### 4.3 보조 품질 기준선
| 항목 | 실행 결과 | 비고 |
|---|---|---|
| `npm.cmd run type-check` | PASS | `tsc --noEmit` 통과 |
| `npm.cmd run lint` | FAIL | `app/workbench/page.tsx` 2건 (`no-unused-vars`, `no-assign-module-variable`) |

### 4.4 lint 이슈 처리 방침(재판정 반영)
- 결정: lint 2건은 즉시 수정하지 않고 이슈 발생 시점에 수정
- 판정 영향: Phase 0 완료 판정의 비차단 항목(운영 리스크로만 관리)

### 4.5 의도 범위 산출물 포함 확인
- `public/wasm/VTK-Workbench.js`, `public/wasm/VTK-Workbench.wasm`:
  - 빌드 성공 시 생성/갱신되는 정상 산출물로 Phase 0 범위에 포함
- `emsdk.bat`:
  - 개발 환경 복구 결과물(emsdk 환경 로딩 헬퍼)로 Phase 0 범위에 포함

## 5. Phase 0 산출물 생성 현황
- 완료:
  - `docs/refactoring/phase0/baseline_manifest_20260325.md`
  - `docs/refactoring/phase0/smoke_checklist.md`
  - `docs/refactoring/phase0/dependency_baseline_report.md`
  - `docs/refactoring/phase0/go_no_go_phase1.md`
  - `docs/legacy/atoms-refactory/README.md`
  - `scripts/refactoring/check_phase0_dependency_baseline.ps1`
  - `emsdk.bat` (환경 복구 산출물)
- 로그:
  - `docs/refactoring/phase0/logs/build_20260327_1730.txt`
  - `docs/refactoring/phase0/logs/build_20260327_1734_cmake_build.txt`
  - `docs/refactoring/phase0/logs/build_20260327_1740_docker.txt`
  - `docs/refactoring/phase0/logs/build_20260327_1645_emsdk_success_attempt.txt`

## 6. 결론
- W0-1(기준선 참조점) 완료
- W0-2(빌드 기준선) 성공 로그 확보 완료(emsdk 경로)
- W0-3/W0-4/W0-5/W0-6은 해당 문서에 반영 완료
