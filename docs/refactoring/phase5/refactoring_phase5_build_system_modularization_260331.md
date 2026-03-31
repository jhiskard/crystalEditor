# Phase 5 세부 작업계획서: 빌드 시스템 모듈화

작성일: `2026-03-31 (KST)`  
최종 업데이트: `2026-03-31 (Phase 1 양식 재정리)`  
대상 범위: 루트 `CMakeLists.txt`, `webassembly/cmake/modules/*`, 모듈별 소스 등록 경계  
진행 상태: `착수 전 계획 확정`

## 0. Phase 4 종료 반영사항

### 0.1 진입 판정
- 판정: **GO**
- 근거 문서:
  - `docs/refactoring/phase4/dependency_gate_report.md`
  - `docs/refactoring/phase4/go_no_go_phase5.md`

### 0.2 착수 기준선(Phase 4 종료 시점)
| 항목 | 기준값 | 출처 |
|---|---:|---|
| 루트 `CMakeLists.txt` 라인 수 | 296 | 코드 기준선 |
| 빌드 소스 등록 방식 | 단일 `add_executable` 중심 | 착수 시점 빌드 스크립트 |
| 모듈 CMake 파일 수 | 0 | 착수 시점 파일 구조 |

### 0.3 Phase 5 범위 확정
- Phase 5 핵심:
  - 모듈 단위 CMake 분할
  - 소스 소유권 정리
  - 링크 경계 명확화
- 컴파일 정책:
  - 개별 WBS 단계에서는 컴파일 테스트를 수행하지 않음
  - 모든 WBS 완료 후 종료 게이트에서 컴파일 테스트 1회 수행

## 1. Phase 5 목표

### 목표
- 빌드 시스템을 모듈 단위로 구조화하여 변경 영향 범위를 줄이고 유지보수성을 높인다.

### 도달 결과물
1. `wb_core`, `wb_render`, `wb_mesh`, `wb_atoms`, `wb_io`, `wb_ui` 모듈 구성
2. 루트 `CMakeLists.txt`의 오케스트레이션 역할 단순화
3. 파일-모듈 소유권 문서화 및 정적 점검 스크립트 제공

### 비목표(Phase 5에서 하지 않음)
- 런타임 기능 리팩토링(Phase 4까지 범위)
- 테스트 프레임워크 체계 개편(Phase 6)

## 2. 현재 문제 요약(착수 근거)
1. 단일 CMake 타깃에 소스가 집중되어 변경 파급 범위가 크다.
2. 소스 파일의 모듈 소유권이 명확하지 않아 책임 추적이 어렵다.
3. 링크 경계(`PUBLIC/PRIVATE`)가 혼재되어 경계 누수가 발생하기 쉽다.

## 3. 작업 원칙
1. 동작 보존 우선(behavior-preserving)
2. 빌드 블록을 점진 분리하고 WBS 단위 추적
3. WBS 1개 완료마다 로컬 `push` 단위 커밋
4. 컴파일 테스트는 WBS별 미수행, 최종 게이트 1회 수행
5. 원격 저장소 동기화는 Phase 5 종료 판정 이후 별도 수행

## 4. 작업 단위(WBS)

## W0. 착수/브랜치 셋업
### 작업
- `refactor/phase5-build-system-modularization` 브랜치 생성
- 산출 디렉터리 준비(`docs/refactoring/phase5`, `logs`)

### 완료 기준
- 브랜치/산출 구조 준비 완료

## W1. CMake Baseline 인벤토리
### 작업
- 루트 CMake 소스 목록/링크/외부 의존성 인벤토리 작성
- 파일별 모듈 소유권 1차 분류

### 영향 파일(예상)
- `CMakeLists.txt`
- `docs/refactoring/phase5/logs/cmake_inventory_phase5_latest.md`

### 완료 기준
- 미분류 소스 없이 1차 소유권 표 완성

## W2. 모듈 타깃 설계 및 스캐폴딩
### 작업
- 모듈 CMake 파일 생성
  - `webassembly/cmake/modules/wb_core.cmake`
  - `webassembly/cmake/modules/wb_render.cmake`
  - `webassembly/cmake/modules/wb_mesh.cmake`
  - `webassembly/cmake/modules/wb_atoms.cmake`
  - `webassembly/cmake/modules/wb_io.cmake`
  - `webassembly/cmake/modules/wb_ui.cmake`
- 타깃 스켈레톤(`target_sources`, `target_include_directories`, `target_link_libraries`) 배치

### 완료 기준
- 모듈 파일 구조 생성 및 초기 링크 골격 완성

## W3. 소스 리스트 모듈 이관
### 작업
- 루트 `CMakeLists.txt`의 대형 소스 목록을 모듈 파일로 이동
- 루트는 모듈 include + 최종 executable 링크 중심으로 단순화

### 영향 파일(예상)
- `CMakeLists.txt`
- `webassembly/cmake/modules/*.cmake`

### 완료 기준
- 중복/누락 없이 모듈별 소스 등록 완료

## W4. 의존성/링크 경계 정리
### 작업
- 모듈 간 링크 방향 정렬(`wb_ui -> wb_atoms -> wb_mesh -> wb_render` 등)
- `PUBLIC/PRIVATE` 범위 정리
- 순환 참조 위험 지점 문서화

### 완료 기준
- 링크 경계와 문서가 일치

## W5. 정적 게이트 스크립트/리포트
### 작업
- 정적 점검 스크립트 작성
  - `scripts/refactoring/check_phase5_build_modules.ps1` (예정)
- 누락/중복/경계 위반 점검 결과 기록

### 완료 기준
- 재현 가능한 정적 점검 루틴 확립

## W6. Gate 문서화/종료 판정 준비
### 작업
- `docs/refactoring/phase5/dependency_gate_report.md` 작성
- `docs/refactoring/phase5/go_no_go_phase6.md` 작성
- W0~W5 완료 후 컴파일 테스트 1회 수행 및 결과 기록

### 완료 기준
- Phase 5 종료 판정 가능한 문서 패키지 완성
- 최종 컴파일 테스트 결과 기록 완료

## 5. 상세 일정(권장)
- Day 1: W0~W1
- Day 2: W2
- Day 3: W3
- Day 4: W4
- Day 5: W5
- Day 6: W6(최종 컴파일 1회 포함)

예상 공수: 2~3 MD

## 6. 파일별 변경 계획

### 신규 파일
- `webassembly/cmake/modules/wb_core.cmake`
- `webassembly/cmake/modules/wb_render.cmake`
- `webassembly/cmake/modules/wb_mesh.cmake`
- `webassembly/cmake/modules/wb_atoms.cmake`
- `webassembly/cmake/modules/wb_io.cmake`
- `webassembly/cmake/modules/wb_ui.cmake`
- `scripts/refactoring/check_phase5_build_modules.ps1`

### 주요 수정 파일
- `CMakeLists.txt`
- `docs/refactoring/phase5/dependency_gate_report.md`
- `docs/refactoring/phase5/go_no_go_phase6.md`

## 7. 커밋/동기화 전략
1. 커밋 1: W0 브랜치/디렉터리 셋업
2. 커밋 2: W1 인벤토리
3. 커밋 3: W2 모듈 스캐폴딩
4. 커밋 4: W3 소스 이관
5. 커밋 5: W4 링크 경계 정리
6. 커밋 6: W5 정적 게이트
7. 커밋 7: W6 게이트 문서 + 최종 컴파일 1회 결과

동기화 규칙:
- 각 커밋 종료 시 로컬 저장소 기준 `local push` 수행
- 원격 저장소 동기화는 Phase 5 범위에서 수행하지 않음

## 8. 리스크 및 대응
1. 리스크: 소스 등록 누락/중복
   - 대응: 정적 스크립트로 중복/누락 점검
2. 리스크: 링크 경계 붕괴
   - 대응: `PUBLIC/PRIVATE` 정책 표준화 및 점검
3. 리스크: 최종 1회 컴파일에서 오류 집중 발생
   - 대응: W1~W5 정적 점검 강화를 통해 사전 결함 제거

## 9. Phase 5 완료(DoD)
- 모듈별 CMake 파일이 생성되고 루트 오케스트레이션 구조로 정리됨
- 소스 소유권 문서/스크립트가 준비됨
- 최종 컴파일 테스트 1회 결과가 게이트 문서에 기록됨
- Phase 6 진입 판정 문서 작성 완료

## 10. 착수 체크리스트
- [x] Phase 4 종료 `GO` 확인
- [x] 컴파일 정책 확인(WBS별 미수행, 최종 1회 수행)
- [ ] `refactor/phase5-build-system-modularization` 브랜치 생성
- [ ] W1 CMake Baseline 인벤토리
- [ ] W2 모듈 타깃 설계/스캐폴딩
- [ ] W3 소스 리스트 모듈 이관
- [ ] W4 의존성/링크 경계 정리
- [ ] W5 정적 게이트 스크립트/리포트
- [ ] W6 Gate 문서화/종료 판정 준비(최종 컴파일 1회 포함)

## 11. Phase 5 클래스/함수 및 빌드 블록 이동 매핑표
> Phase 5는 코드 함수 이동보다 빌드 정의 블록 이동이 핵심이다.  
> 아래 표는 클래스/함수 소유 모듈과 CMake 블록 재배치 계획을 함께 기록한다.

| WBS | 기존 위치 | 클래스/함수 또는 빌드 블록(현행) | 신규 모듈(목표 위치) | 방식 | 비고 |
|---|---|---|---|---|---|
| W1 | `CMakeLists.txt` | 단일 소스 목록 블록 | `docs/refactoring/phase5/logs/cmake_inventory_phase5_latest.md` | 분류 | 파일-모듈 소유권 정의 |
| W2 | `CMakeLists.txt` | `add_executable`용 대형 소스 등록 블록 | `webassembly/cmake/modules/wb_*.cmake` | 이동 | 모듈별 `target_sources` 분리 |
| W2 | `CMakeLists.txt` | include/link 설정 혼합 블록 | 각 `wb_*.cmake` | 이동 | `target_include_directories`, `target_link_libraries` 분리 |
| W3 | `CMakeLists.txt` | atoms 관련 소스 등록 | `wb_atoms.cmake` | 이동 | atoms 소유권 명확화 |
| W3 | `CMakeLists.txt` | io 관련 소스 등록 | `wb_io.cmake` | 이동 | import/parse 소유권 분리 |
| W3 | `CMakeLists.txt` | mesh/model tree 관련 소스 등록 | `wb_mesh.cmake` | 이동 | mesh 소유권 분리 |
| W3 | `CMakeLists.txt` | viewer/render 관련 소스 등록 | `wb_render.cmake` | 이동 | render 소유권 분리 |
| W3 | `CMakeLists.txt` | app/ui 관련 소스 등록 | `wb_ui.cmake`, `wb_core.cmake` | 이동 | 진입/공통 경계 분리 |
| W4 | 각 모듈 CMake | 링크 방향 정의 | 모듈 간 링크 체인 | 정렬 | 순환 참조 방지 |
| W5 | `scripts/refactoring/*` | 모듈 누락/중복/경계 점검 규칙 | `check_phase5_build_modules.ps1` | 신규 | 정적 gate 자동화 |
| W6 | `CMakeLists.txt` + 모듈 파일 | 최종 링크 오케스트레이션 | 루트 + 모듈 조합 | 확정 | 종료 게이트 컴파일 1회 검증 |

## 12. Phase 5 실행 순서(작업자 기준)
1. 현행 CMake 인벤토리/소유권 확정(W1)
2. 모듈 스켈레톤 생성(W2)
3. 소스 등록 이관(W3)
4. 링크 경계 정리(W4)
5. 정적 게이트 자동화(W5)
6. 게이트 문서화 + 최종 컴파일 1회(W6)

## 13. Phase 5 수용 기준(정량)
| 항목 | Baseline | Target | 검증 방식 |
|---|---:|---:|---|
| 루트 `CMakeLists.txt` 라인 수 | 296 | <= 177 | 라인 수 측정 |
| 모듈 CMake 파일 수 | 0 | 6 이상 | 파일 존재 확인 |
| 미분류 소스 수 | N/A | 0 | 인벤토리 리포트 |
| 소스 등록 중복/누락 | N/A | 0 | `check_phase5_build_modules.ps1` |
| 컴파일 테스트 | WBS별 미수행 | 최종 1회 수행/기록 | W6 게이트 로그 |
