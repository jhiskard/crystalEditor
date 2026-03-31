# Phase 6 세부 작업계획서: 테스트/회귀 체계 도입

작성일: `2026-03-31 (KST)`  
최종 업데이트: `2026-03-31 (W0~W6 구현 및 게이트 완료)`  
대상 범위: `webassembly/tests/*`, `tests/e2e/*`, `app/workbench/page.tsx`, `package.json`, `CMakeLists.txt`, `scripts/refactoring/*`  
진행 상태: `종료 판정 자료 준비 완료(Phase 6 DONE / Phase 7 판정 가능)`

## 0. Phase 5 종료 반영사항

### 0.1 진입 판정
- 판정: **GO**
- 근거 문서:
  - `docs/refactoring/phase5/dependency_gate_report.md`
  - `docs/refactoring/phase5/go_no_go_phase6.md`

### 0.2 착수 기준선(Phase 5 종료 시점)
| 항목 | 기준값 | 출처 |
|---|---:|---|
| `webassembly/tests` 디렉터리 | 없음 | `Test-Path webassembly/tests` |
| 전용 test script 수 | 0 | `package.json` |
| C++ 단위 테스트 타깃 수 | 0 | 현행 CMake/소스 트리 |
| Playwright 의존성 | 설치됨 | `package.json` |
| Phase 0 스모크 시나리오 수 | 8 | `docs/refactoring/phase0/smoke_checklist.md` |
| `chgcar_parser.cpp` 라인 수 | 397 | 계획 기준선 |
| `cell_manager.cpp` 라인 수 | 122 | 계획 기준선 |
| `atoms_template.cpp` 라인 수 | 5,360 | 계획 기준선 |

### 0.3 Phase 6 범위 확정
- Phase 6 핵심:
  - 순수 계산/파서 로직에 대한 네이티브 C++ 단위 테스트 경로 구축
  - 브라우저 런타임 회귀를 감시하는 Playwright 스모크 경로 구축
  - 테스트 실행 명령, 로그, 게이트 문서를 Phase 산출물로 정리
- 선행 정리 과제:
  - `AtomsTemplate` 내부 측정 지오메트리 helper를 테스트 가능한 순수 함수 경계로 분리
  - XSF import 검증은 OS 파일 다이얼로그 대신 MemFS/loadArrayBuffer 경로를 사용
- 후속 Phase 이관:
  - 전체 UI 픽셀 회귀 테스트
  - 원격 CI(GitHub Actions 등) 상시 연동

## 1. Phase 6 목표

### 목표
- 핵심 계산과 import 회귀를 로컬에서 반복 가능하고 빠르게 검증할 수 있는 자동 테스트 경로로 전환한다.

### 도달 결과물
1. `webassembly/tests` 기반 C++ 단위 테스트 하네스
2. CHGCAR/XSF 파서, 좌표 변환, 측정 지오메트리 테스트 스위트
3. `/workbench` 기준 import -> tree -> visibility -> render Playwright 스모크
4. `package.json` 테스트 명령과 Phase 6 게이트 로그/문서

### 비목표(Phase 6에서 하지 않음)
- 전체 화면 스냅샷 골든 이미지 체계
- 모든 ImGui 상호작용의 완전 자동화
- 배포 파이프라인 수준의 원격 CI 완성

## 2. 현재 문제 요약(착수 근거)
1. `package.json`에 lint/type-check 외 전용 테스트 실행 경로가 없다.
2. 현행 루트 CMake는 Emscripten executable 중심이어서 순수 계산 로직의 빠른 로컬 검증 경로가 없다.
3. 측정 계산 helper(`calcDistance`, `computeAngleMeasurementGeometry`, `computeDihedralMeasurementGeometry`)가 `atoms_template.cpp` 내부에 남아 있어 단위 테스트 경계가 약하다.
4. Phase 0/4 수동 스모크 기록은 존재하지만 자동 회귀 감시 경로가 없다.
5. 브라우저 import 경로는 파일 다이얼로그 기반으로 사용되지만, 실제 앱에는 `VtkModule.FS.createDataFile + loadArrayBuffer` 기반의 자동화 가능한 경로가 이미 존재한다.

## 3. 작업 원칙
1. 빠른 피드백 우선: 파서/수학 테스트는 브라우저/WASM이 아닌 네이티브 실행 경로로 분리
2. 브라우저 스모크는 최소 핵심 시나리오만 자동화하고, 파일 다이얼로그 대신 MemFS import seam을 사용
3. 테스트 추가를 위해 필요한 seam 추출은 허용하되, 런타임 동작 변경은 금지
4. WBS 종료 단위 로그와 문서를 남겨 Phase 종료 판정 가능 상태까지 끌고 간다
5. 수치 검증은 허용 오차를 명시하고, UI 테스트는 픽셀보다 상태/에러 부재를 우선 확인한다

## 4. 작업 단위(WBS)

## W0. 착수/브랜치 셋업
### 작업
- `refactor/phase6-test-regression-system` 브랜치 준비
- 산출 디렉터리 준비(`docs/refactoring/phase6`, `docs/refactoring/phase6/logs`)
- 기준선 인벤토리 문서 초안 작성

### 완료 기준
- 브랜치 및 산출 구조 준비 완료

## W1. 테스트 하네스/실행 경로 설계
### 작업
- C++ 단위 테스트와 브라우저 스모크를 분리한 실행 구조 확정
- `CMakeLists.txt` 또는 `webassembly/tests/CMakeLists.txt`에 테스트 타깃 진입점 설계
- `package.json`에 `test:cpp`, `test:smoke`, `test:phase6` 명령 추가 계획 반영
- 경량 assertion/runner 방식을 정하고 외부 의존 추가 여부를 최소화

### 영향 파일(예상)
- `CMakeLists.txt`
- `package.json`
- `webassembly/tests/CMakeLists.txt`
- `docs/refactoring/phase6/logs/test_inventory_phase6_latest.md`

### 완료 기준
- 로컬 1회 실행 가능한 테스트 명령 체계가 설계 수준에서 고정됨

## W2. Parser fixture 및 단위 테스트 구축
### 작업
- `webassembly/tests/fixtures/io/`에 CHGCAR/XSF fixture 추가
- CHGCAR 파서 테스트 작성
  - header/원자 개수/grid shape
  - density min/max
  - 실패 입력 처리
- XSF 구조/그리드 테스트 작성
  - `FileIOManager` 경유 fixture 검증 또는 테스트용 parser seam 확보
  - cell vectors, atom count, grid dims, error path 검증

### 영향 파일(예상)
- `webassembly/tests/io/chgcar_parser_test.cpp`
- `webassembly/tests/io/xsf_parser_test.cpp`
- `webassembly/tests/fixtures/io/*`
- `webassembly/src/atoms/infrastructure/file_io_manager.{h,cpp}` (seam 보강 시)

### 완료 기준
- CHGCAR/XSF 대표 fixture가 자동 검증 경로에 포함됨

## W3. 좌표 변환 단위 테스트 구축
### 작업
- `calculateInverseMatrix`, `cartesianToFractional`, `fractionalToCartesian` 테스트 작성
- 직교 셀, 비직교 셀, round-trip 변환, 허용 오차 검증
- 특이 행렬/비정상 입력의 현재 동작을 문서화하고 필요한 guard를 최소 보강

### 영향 파일(예상)
- `webassembly/tests/atoms/cell_transform_test.cpp`
- `webassembly/src/atoms/domain/cell_manager.{h,cpp}`

### 완료 기준
- 좌표 변환 수치 검증이 허용 오차와 함께 자동화됨

## W4. 측정 지오메트리 seam 분리 및 단위 테스트
### 작업
- `atoms_template.cpp` 상단의 순수 계산 helper를 별도 모듈로 추출
  - `calcDistance`
  - `computeAngleMeasurementGeometry`
  - `computeDihedralMeasurementGeometry`
- VTK actor 생성과 수학 계산 경계를 분리
- distance/angle/dihedral 정상/퇴화 케이스 테스트 작성

### 영향 파일(예상)
- `webassembly/src/atoms/domain/measurement_geometry.{h,cpp}`
- `webassembly/src/atoms/atoms_template.cpp`
- `webassembly/tests/atoms/measurement_geometry_test.cpp`

### 완료 기준
- 측정 계산 검증이 `VtkViewer::Instance()` 없이 실행 가능해짐

## W5. 브라우저 스모크 자동화
### 작업
- Playwright 설정 파일 추가 및 실행 명령 연결
- `/workbench` 기준 핵심 회귀 시나리오 자동화
  - XSF 구조 import
  - XSF grid 또는 CHGCAR import
  - tree 반영 확인
  - visibility 토글 후 렌더 반영 확인
- `app/workbench/page.tsx`의 `FS.createDataFile + loadArrayBuffer` 경로를 테스트 seam으로 사용
- 콘솔 에러/런타임 예외 감시 추가

### 영향 파일(예상)
- `playwright.config.ts`
- `tests/e2e/workbench-smoke.spec.ts`
- `app/workbench/page.tsx` (테스트용 ready signal/selector 보강 시)

### 완료 기준
- 핵심 import/visibility/render 회귀가 브라우저에서 자동 감시됨

## W6. Gate 검사/문서화
### 작업
- Phase 6 실행 스크립트 작성
  - `scripts/refactoring/check_phase6_test_regression.ps1`
- 테스트 로그 수집
  - `docs/refactoring/phase6/logs/unit_test_phase6_latest.txt`
  - `docs/refactoring/phase6/logs/smoke_phase6_latest.txt`
  - `docs/refactoring/phase6/logs/test_inventory_phase6_latest.md`
- 게이트 리포트 및 Go/No-Go 문서 작성

### 완료 기준
- Phase 7 착수 판정 가능한 테스트/회귀 문서 패키지 완성

## 5. 상세 일정(권장)
- Day 1: W0~W1
- Day 2: W2
- Day 3: W3
- Day 4: W4
- Day 5: W5
- Day 6: W6

예상 공수: 5~6 MD

## 6. 파일별 변경 계획

### 신규 파일
- `docs/refactoring/phase6/refactoring_phase6_test_regression_system_260331.md`
- `webassembly/tests/CMakeLists.txt`
- `webassembly/tests/test_main.cpp`
- `webassembly/tests/io/chgcar_parser_test.cpp`
- `webassembly/tests/io/xsf_parser_test.cpp`
- `webassembly/tests/atoms/cell_transform_test.cpp`
- `webassembly/tests/atoms/measurement_geometry_test.cpp`
- `webassembly/tests/fixtures/io/*`
- `webassembly/src/atoms/domain/measurement_geometry.{h,cpp}`
- `playwright.config.ts`
- `tests/e2e/workbench-smoke.spec.ts`
- `scripts/refactoring/check_phase6_test_regression.ps1`

### 주요 수정 파일
- `CMakeLists.txt`
- `package.json`
- `app/workbench/page.tsx`
- `webassembly/src/atoms/atoms_template.cpp`
- `webassembly/src/atoms/domain/cell_manager.{h,cpp}`
- `webassembly/src/atoms/infrastructure/file_io_manager.{h,cpp}`

## 7. 커밋/동기화 전략
1. 커밋 1: W0 셋업/기준선
2. 커밋 2: W1 하네스/스크립트 뼈대
3. 커밋 3: W2 Parser 테스트
4. 커밋 4: W3 Cell transform 테스트
5. 커밋 5: W4 Measurement geometry seam + 테스트
6. 커밋 6: W5 Playwright smoke
7. 커밋 7: W6 Gate 문서

## 8. 리스크 및 대응
1. 리스크: 네이티브 테스트와 WASM 런타임의 환경 차이
   - 대응: 순수 계산/파서는 네이티브, import/render 회귀는 Playwright로 분리 검증
2. 리스크: 파일 다이얼로그 자동화의 불안정성
   - 대응: `loadArrayBuffer` 기반 MemFS import seam만 사용
3. 리스크: 측정 helper 분리 중 UI/VTK 경계 재결합
   - 대응: 수학 계산 모듈은 VTK 타입 비의존 규칙 고정
4. 리스크: XSF 파서가 파일 경로 중심이라 테스트 fixture 주입이 번거로움
   - 대응: 테스트용 임시 파일 또는 최소 seam 추가로 API 확장 범위를 제한

## 9. Phase 6 완료(DoD)
- `webassembly/tests` 기반 C++ 테스트 스위트가 추가됨
- CHGCAR/XSF 파서, 좌표 변환, 측정 지오메트리 테스트가 모두 PASS
- `/workbench` import -> tree -> visibility -> render 스모크가 자동화됨
- `package.json`에서 Phase 6 테스트 명령 1회 실행 경로가 제공됨
- 게이트 리포트와 Phase 7 Go/No-Go 문서가 준비됨

## 10. 착수 체크리스트
- [x] Phase 5 종료 `GO` 확인
- [x] W0 착수/브랜치 셋업
- [x] W1 테스트 하네스/실행 경로 설계
- [x] W2 Parser fixture 및 단위 테스트 구축
- [x] W3 좌표 변환 단위 테스트 구축
- [x] W4 측정 지오메트리 seam 분리 및 단위 테스트
- [x] W5 브라우저 스모크 자동화
- [x] W6 Gate 검사/문서화

## 11. Phase 6 클래스/함수-테스트 매핑표
> 표기  
> - 추가: 기존 코드에 대한 신규 테스트만 추가  
> - 분리+추가: 테스트 가능 경계를 만들기 위해 helper를 분리한 뒤 테스트 추가

| WBS | 기존 위치 | 클래스/함수(현행) | 신규 모듈(목표 위치) | 방식 | 비고 |
|---|---|---|---|---|---|
| W2 | `webassembly/src/atoms/infrastructure/chgcar_parser.cpp` | `ChgcarParser::parse`, `parseHeader`, `parseAtomPositions`, `parseDensityData` | `webassembly/tests/io/chgcar_parser_test.cpp` | 추가 | 대표 CHGCAR fixture 검증 |
| W2 | `webassembly/src/atoms/infrastructure/file_io_manager.cpp` | `loadXSFFile`, `load3DGridXSFFile`, 내부 XSF parse 경로 | `webassembly/tests/io/xsf_parser_test.cpp` | 추가 | 구조/그리드 XSF 검증 |
| W3 | `webassembly/src/atoms/domain/cell_manager.cpp` | `calculateInverseMatrix`, `cartesianToFractional`, `fractionalToCartesian` | `webassembly/tests/atoms/cell_transform_test.cpp` | 추가 | 직교/비직교/round-trip |
| W4 | `webassembly/src/atoms/atoms_template.cpp` | `calcDistance` | `webassembly/src/atoms/domain/measurement_geometry.cpp` + `webassembly/tests/atoms/measurement_geometry_test.cpp` | 분리+추가 | 순수 거리 계산 분리 |
| W4 | `webassembly/src/atoms/atoms_template.cpp` | `computeAngleMeasurementGeometry` | `webassembly/src/atoms/domain/measurement_geometry.cpp` + `webassembly/tests/atoms/measurement_geometry_test.cpp` | 분리+추가 | angle arc/text geometry 검증 |
| W4 | `webassembly/src/atoms/atoms_template.cpp` | `computeDihedralMeasurementGeometry` | `webassembly/src/atoms/domain/measurement_geometry.cpp` + `webassembly/tests/atoms/measurement_geometry_test.cpp` | 분리+추가 | dihedral helper geometry 검증 |
| W5 | `app/workbench/page.tsx` | `FS.createDataFile`, `loadArrayBuffer`, pending import 처리 경로 | `tests/e2e/workbench-smoke.spec.ts` | 추가 | 파일 다이얼로그 없는 import 자동화 |
| W5 | `docs/refactoring/phase0/smoke_checklist.md` | S1/S2/S4/S5 핵심 시나리오 | `tests/e2e/workbench-smoke.spec.ts` | 추가 | 수동 스모크 일부 자동화 |

## 12. Phase 6 실행 순서(작업자 기준)
1. 하네스와 실행 명령을 먼저 고정한다(W0~W1)
2. 파서/수학처럼 빠른 로직 테스트를 먼저 확보한다(W2~W4)
3. 브라우저 회귀는 MemFS import seam 기반으로 최소 시나리오부터 자동화한다(W5)
4. 로그, 게이트, 종료 판정 문서를 마지막에 묶는다(W6)

## 13. Phase 6 수용 기준(정량)
| 항목 | Baseline | Target | 검증 방식 |
|---|---:|---:|---|
| `webassembly/tests` 디렉터리 | 없음 | 존재 | 디렉터리/파일 확인 |
| C++ 테스트 파일 수 | 0 | 4 이상 | 소스 인벤토리 |
| Playwright smoke spec 수 | 0 | 1 이상 | 파일 확인 |
| 전용 test script 수 | 0 | 3 이상 | `package.json` |
| 자동화된 핵심 스모크 시나리오 수 | 0 | 4 이상 | smoke spec/log |
| Phase 6 게이트 명령 | 없음 | 1개 제공 | `check_phase6_test_regression.ps1` |
