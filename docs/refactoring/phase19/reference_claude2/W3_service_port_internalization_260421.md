# Phase 19 / W3 — Feature 서비스 포트/Adapter 내재화

작성일: `2026-04-21 (KST)`
상위 계획서: `docs/refactoring/refactoring_plan_phase19_legacy_full_dismantle_claude2_260421.md`
검증보고서 근거: 5.1 R1, R2 (법적 격리 → 물리 소거 필요), 5.2 관련 권고
우선순위: **Medium**
예상 소요: 2 ~ 3일 (3개 feature 병렬 가능)
선행 의존: W1 (friend 제거 — io 영역 안정화 직전에 필요), W2 (measurement 서명 안정화)
후속 작업: W4 (io/render legacy 제거), W5 (AtomsTemplate 해체)

---

## 1. 배경

Phase 10~12에서 각 feature(structure, measurement, density)는 `infrastructure/legacy/legacy_*_service_port.{h,cpp}`를 도입해 `AtomsTemplate` 접근을 **포트/어댑터**로 격리했다. 이는 Phase 18까지 "격리 유지" 판정을 얻었지만, Phase 19는 격리를 영구적으로 **물리 소거**로 승격한다.

핵심 원칙: feature의 `application/*_service.cpp`는 더 이상 legacy adapter를 경유하지 않고 **도메인 Repository/Store를 직접 소유/호출**한다. 이로써:

1. `infrastructure/legacy/` 디렉터리 소멸
2. `LegacyAtomsRuntime()` 호출 경로 일부 소멸 (W3 스코프 내 모든 호출이 소멸되는 것은 아님; W4, W5에서 나머지 소멸)
3. 도메인 서비스의 단일 소유자가 `WorkbenchRuntime`이 됨 (Phase 7 composition root 강화)

## 2. 현재 상태

| Feature | 포트 파일 | 공통 위반 | LOC |
|---|---|---|---:|
| structure | `structure/infrastructure/legacy/legacy_structure_service_port.{h,cpp}` | `LegacyAtomsRuntime()` 내부 호출 | 105 |
| measurement | `measurement/infrastructure/legacy/legacy_measurement_service_port.{h,cpp}` | `LegacyAtomsRuntime()` 내부 호출 | 148 |
| density | `density/infrastructure/legacy/legacy_density_service_port.{h,cpp}` | `LegacyAtomsRuntime()` 내부 호출 | 104 |
| **합계** | 3폴더 6파일 | | **357** |

```
┌──────────────────────┐
│ feature/application/ │
│   *_service.cpp      │  ← 현재: 추상 port 주입 + legacy adapter 구현
└──────────────────────┘
          │ (port)
          ▼
┌──────────────────────┐
│ infrastructure/legacy/│ ← 제거 대상
│ legacy_*_service_port│
│   .cpp               │
└──────────────────────┘
          │
          ▼
   LegacyAtomsRuntime() → AtomsTemplate
```

## 3. 목표 상태

```
┌──────────────────────┐
│ feature/application/ │
│   *_service.cpp      │  ← 도메인 Repository/Store를 생성자 주입으로 직접 소유
└──────────────────────┘
          │ (직접 호출)
          ▼
┌──────────────────────┐
│ feature/domain/      │
│   *_repository       │
│   *_store            │
└──────────────────────┘
```

## 4. 서브 단계

### W3.1 — StructureService 내재화

1. **인벤토리**: `structure/application/structure_service.cpp`가 `LegacyStructureServicePort` 경유로 호출하는 메서드 전수 리스트화.
   산출물: `docs/refactoring/phase19/logs/structure_port_surface_phase19_latest.md`
2. **도메인 서비스 직접 호출로 재배선**:
   - Structure Repository 접근 → `structure/domain/structure_repository`
   - 선택 상태 → `structure/domain/structure_selection_store`
   - BZ 그리드/특수점 → `structure/application/structure_lifecycle_service` (W5.4에서 확장)
3. **`StructureServicePort` 추상 인터페이스 처리 결정**:
   - 기준: `WorkbenchRuntime`의 테스트 double 주입 수요가 있으면 유지, 오직 legacy adapter만 구현하면 제거
   - W3.1 종료 시점 결정을 `docs/refactoring/phase19/logs/port_retention_decisions_W3_latest.md`에 기록
4. **파일 삭제**:
   - `structure/infrastructure/legacy/legacy_structure_service_port.h`
   - `structure/infrastructure/legacy/legacy_structure_service_port.cpp`
   - 디렉터리 `structure/infrastructure/legacy/` 제거
5. **CMake**: `webassembly/cmake/modules/wb_structure.cmake`에서 엔트리 제거.
6. **회귀**: structure 단위/통합 테스트 PASS + e2e smoke PASS.

### W3.2 — MeasurementService 내재화

1. **선행**: W2 완료 (MeasurementService 서명이 `PickedAtomInfo` 기반으로 안정화된 상태).
2. **재배선**:
   - `measurement/application/measurement_service.cpp`가 `MeasurementStore`(도메인) 및 `StructureRepository`에 직접 접근
   - overlay 렌더 트리거는 `render/application/render_gateway`로 직접 dispatch
3. **포트 유지/삭제 결정**: W3.1과 동일 기준.
4. **파일 삭제**: `measurement/infrastructure/legacy/legacy_measurement_service_port.{h,cpp}` + 디렉터리.
5. **CMake**: `wb_measurement.cmake` 갱신.
6. **회귀**: measurement 단위/e2e PASS.

### W3.3 — DensityService 내재화

1. **재배선**: `density/application/density_service.cpp`가 `DensityRepository`(신규 또는 기존) 직접 접근.
2. **로직 이관 주의**: charge density의 VTK 측면(그리드 렌더)은 `density/presentation/charge_density_controller`로 이관되는데, 이는 W5.3의 범위. W3.3에서는 port 소멸만 처리하고 그리드 렌더 로직은 `LegacyAtomsRuntime()`을 경유하는 과도기 구현을 유지할 수 있음.
3. **파일 삭제**: `density/infrastructure/legacy/legacy_density_service_port.{h,cpp}` + 디렉터리.
4. **CMake**: `wb_density.cmake` 갱신.
5. **회귀**: charge density 로드/이소표면 표시 e2e PASS.

## 5. 공통 절차

각 서브 단계(W3.1~W3.3)에서 공통적으로 수행:

1. 포트 메서드 표면 (public) 전수 리스트화
2. 각 메서드의 "현재 구현"이 `LegacyAtomsRuntime()` 경유인지, 이미 도메인 호출인지 분류
3. `LegacyAtomsRuntime()` 경유 메서드에 대해 도메인 경로 대체 가능성 평가:
   - 100% 대체 가능 → 즉시 재배선
   - 부분 대체 — 일부 로직이 `AtomsTemplate`에 남음 → W5.x에서 후속 처리 필요 메모 기록 (port 파일은 제거하되 `*_service.cpp` 내부에서 `LegacyAtomsRuntime()` 직접 호출로 퇴행시킨다 — 이는 W5에서 자연 소멸)
4. 도메인 경로가 아예 존재하지 않음 → `domain/` 레이어에 필요한 Repository/Store 메서드 선 추가

## 6. 파일 변경 명세 (요약)

| 경로 | 변경 유형 |
|---|---|
| `webassembly/src/structure/infrastructure/legacy/**` | **전량 삭제** |
| `webassembly/src/measurement/infrastructure/legacy/**` | **전량 삭제** |
| `webassembly/src/density/infrastructure/legacy/**` | **전량 삭제** |
| `webassembly/src/structure/application/structure_service.{h,cpp}` | 수정 (도메인 직접 호출) |
| `webassembly/src/measurement/application/measurement_service.{h,cpp}` | 수정 |
| `webassembly/src/density/application/density_service.{h,cpp}` | 수정 |
| `webassembly/cmake/modules/wb_structure.cmake` | 수정 |
| `webassembly/cmake/modules/wb_measurement.cmake` | 수정 |
| `webassembly/cmake/modules/wb_density.cmake` | 수정 |
| `webassembly/src/workspace/runtime/workbench_runtime.cpp` | 수정 (포트 엔트리 제거) |
| `docs/refactoring/phase19/logs/structure_port_surface_phase19_latest.md` | **신규** |
| `docs/refactoring/phase19/logs/measurement_port_surface_phase19_latest.md` | **신규** |
| `docs/refactoring/phase19/logs/density_port_surface_phase19_latest.md` | **신규** |
| `docs/refactoring/phase19/logs/port_retention_decisions_W3_latest.md` | **신규** |
| `docs/refactoring/phase19/logs/dependency_gate_report_W3.md` | **신규** |

## 7. 완료 기준 (DoD)

| 항목 | 측정 | 기준 |
|---|---|---|
| `webassembly/src/structure/infrastructure/legacy/` 존재 | find | 없음 |
| `webassembly/src/measurement/infrastructure/legacy/` 존재 | find | 없음 |
| `webassembly/src/density/infrastructure/legacy/` 존재 | find | 없음 |
| `*_service.cpp` 파일의 `LegacyStructureServicePort`/`LegacyMeasurementServicePort`/`LegacyDensityServicePort` 심볼 참조 | grep | 0 |
| 각 feature 단위 테스트 | 해당 테스트 타깃 | PASS |
| e2e smoke | Playwright | PASS |
| CMake 빌드 | `npm run build-wasm:debug` | PASS |

## 8. 리스크와 완화

| 리스크 | 영향 | 완화책 |
|---|---|---|
| 도메인 레이어(`*_repository`, `*_store`)가 필요한 메서드를 충분히 노출하지 않음 | 재배선 불가 | 각 서브 단계 착수 전 인벤토리에서 gap을 식별, domain 메서드 선 추가 |
| `*_service.cpp`에서 `LegacyAtomsRuntime()`으로 퇴행 (W5 이전 과도기) | 일시적 DoD 미달 | W5에서 자연 소멸한다는 주석 + 추적 ID `P19-W3-to-W5-bridge` 기입 |
| Port 테스트 더블 주입 채널 상실 | CI 유닛 커버리지 감소 | `*_service_port.h` 추상 유지 여부 파일별 결정, 유지 시 `docs/refactoring/phase19/logs/port_retention_decisions_W3_latest.md`에 명시 |

## 9. 산출물

- 삭제 파일 6개 + 폴더 3개
- 수정 파일 9~12개
- 로그 문서 4~5종
- `docs/refactoring/phase19/logs/dependency_gate_report_W3.md`

## 10. Git 커밋 템플릿 (서브 단계당 1커밋)

```
refactor(structure): phase19/W3.1 — internalize structure service port

- Remove structure/infrastructure/legacy/*
- structure_service.cpp now depends on StructureRepository directly
- Retain StructureServicePort abstract as test double channel (see retention decisions log)

metrics:
  structure/infrastructure/legacy/ files: 2 -> 0
  LegacyStructureServicePort references: N -> 0
  structure_service.cpp LegacyAtomsRuntime() usage: N -> M (residue, resolved in W5.1/W5.4)

verif:
  structure unit tests: PASS
  e2e workbench-smoke (structure): PASS
```

유사 템플릿을 W3.2, W3.3에 적용.

---

*W3 완료 시 `infrastructure/legacy/` 라는 이름의 폴더가 3개 feature에서 소멸한다. 이는 "각 feature는 스스로의 도메인 레이어를 소유한다"는 아키텍처 본래 의도의 복원이다.*
