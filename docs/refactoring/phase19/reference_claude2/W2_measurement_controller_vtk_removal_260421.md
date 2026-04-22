# Phase 19 / W2 — `measurement_controller.cpp` VTK 토큰 제거

작성일: `2026-04-21 (KST)`
상위 계획서: `docs/refactoring/refactoring_plan_phase19_legacy_full_dismantle_claude2_260421.md`
검증보고서 근거: 5.1 R (Phase 15 DoD 부분 달성), 5.2 Medium 권고
우선순위: **Medium**
예상 소요: 0.5 ~ 1일
선행 의존: W0
후속 연동: W5.2 (`AtomsTemplate::HandleMeasurementClickByPicker` 최종 소멸)

---

## 1. 배경

Phase 15 DoD는 "`render` 모듈 외부의 VTK 직접 조작 0"이었다. 독립 검증보고서는 해당 DoD를 "부분 달성"으로 판정했다. 유일한 위반은 `measurement/application/measurement_controller.cpp:43`에 위치한 `AtomsTemplate::HandleMeasurementClickByPicker(vtkActor* actor, double pickPos[3])` 정의부로, measurement application 레이어가 `vtkActor*`를 직접 파라미터로 받고 있다. 이 신호는 render 모듈에서 measurement 모듈로 포인터가 건너온다는 의미이며, Phase 15 포트 격리 원칙을 희석한다.

W2는 이 단일 위반을 값 객체(`PickedAtomInfo`) 경유로 교체하여 Phase 15 DoD를 최종 달성 상태로 전환한다.

## 2. 현재 상태

| 항목 | 값 |
|---|---|
| `measurement/` 하위 `vtkActor` 참조 파일 | 1 (`measurement_controller.cpp`) |
| 참조 라인 | `AtomsTemplate::HandleMeasurementClickByPicker(vtkActor* actor, double pickPos[3])` (line 43) |
| 호출부 | `render/application/viewer_interaction_controller.cpp` |
| `render/application/render_gateway.h`의 `ResolvePickedAtom` API 존재 여부 | 확인 필요 (W0에서 그립하여 기록) |

### 2.1 호출 체인

```
viewer_interaction_controller.cpp  (render/application)
  ├─ [click event]
  ├─ picker에게 actor, pickPos 획득
  └─ AtomsTemplate::HandleMeasurementClickByPicker(actor, pickPos)
        ├─ actor로부터 structureId, atomId 역참조
        ├─ MeasurementService에게 hit 정보 전달
        └─ overlay 갱신 트리거
```

목표 체인:

```
viewer_interaction_controller.cpp  (render/application)
  ├─ [click event]
  ├─ picker → RenderGateway::resolvePickedAtom(pickPos) → PickedAtomInfo
  └─ MeasurementService::handlePickerClick(PickedAtomInfo)
        ├─ structureId, atomId, position을 값으로 수신
        └─ overlay 갱신 트리거
```

## 3. 설계

### 3.1 신규 값 객체

```cpp
// webassembly/src/render/application/picked_atom_info.h (신규)
namespace wb::render::application {

struct PickedAtomInfo {
    bool hit = false;                    // picker가 atom에 적중했는지
    int structureId = -1;                // Repository 내 structure index
    int atomId = -1;                     // structure 내 atom index
    std::array<double, 3> pickPosition{}; // world coords
    std::array<double, 3> atomPosition{}; // atom 중심 world coords
};

} // namespace wb::render::application
```

### 3.2 `RenderGateway` API

```cpp
// webassembly/src/render/application/render_gateway.h (기존 확장)
namespace wb::render::application {

class RenderGateway {
public:
    // 신규 또는 기존이 있으면 서명 맞춤
    PickedAtomInfo resolvePickedAtom(const std::array<double, 3>& pickPos) const;
    // 또는 actor 인자 기반이 이미 있으면 그것을 내부에서 사용하는 얇은 래퍼 도입
};

} // namespace wb::render::application
```

구현부(`render/infrastructure/vtk_render_gateway.cpp`)는 VTK picker 재실행 또는 이미 수행된 pick 결과 캐시를 사용해 `PickedAtomInfo`를 반환한다. **`vtkActor*`는 render 경계 내부에 절대 잔존**해야 하며 measurement/shell/workspace/io 어디로도 누설되지 않는다.

### 3.3 `MeasurementService` 서명 변경

```cpp
// 기존
void handlePickerClick(vtkActor* actor, double pickPos[3]);
// 변경 후
void handlePickerClick(const wb::render::application::PickedAtomInfo& hit);
```

### 3.4 `AtomsTemplate::HandleMeasurementClickByPicker`의 운명

본 W2에서는 **서명만 `PickedAtomInfo` 경유로 변경**하여 일단 Phase 15 게이트를 PASS시키고, 함수 자체의 소멸은 W5.2에서 `MeasurementService::handlePickerClick(PickedAtomInfo)`로 완전 이관된다. 따라서 W2의 스코프는 "VTK 토큰 제거"로 한정하고, facade 해체는 W5.2로 분리한다.

## 4. 실행 순서

1. **인벤토리 확정**: `measurement/` 하위 `vtk*` 토큰 grep → `docs/refactoring/phase19/logs/measurement_vtk_token_inventory_phase19_latest.md`
2. **`PickedAtomInfo` 도입**: `render/application/picked_atom_info.h` 신규.
3. **`RenderGateway::resolvePickedAtom` 추가**:
   - `render/application/render_gateway.h` — 인터페이스 선언
   - `render/infrastructure/vtk_render_gateway.{h,cpp}` — VTK picker 기반 구현
4. **`MeasurementService` 서명 변경**:
   - `measurement/application/measurement_service.{h,cpp}` — `handlePickerClick` 매개변수 교체
5. **`AtomsTemplate::HandleMeasurementClickByPicker` 정의부 서명 교체** (`measurement/application/measurement_controller.cpp`):
   - `vtkActor*` 파라미터 제거, `PickedAtomInfo` 파라미터 수용
   - 내부 구현은 W5.2 전까지 `AtomsTemplate` 컨텍스트 보존
6. **호출부 조정**:
   - `render/application/viewer_interaction_controller.cpp` — `RenderGateway::resolvePickedAtom` 호출 후 값 전달
7. **`#include <vtkActor.h>` 제거**: measurement 모듈 전체에서 VTK 헤더 include 제거.
8. **CMake 링크 정리**: `webassembly/cmake/modules/wb_measurement.cmake` 에서 `VTK::RenderingCore` 링크 의존 검토 (여전히 필요한지 확인, 불필요하면 제거).
9. **게이트 재실행**:
   - `check:phase15:render-platform-isolation` PASS 확인
10. **회귀 테스트**:
    - `tests/e2e/workbench-smoke.spec.ts` — measurement 2점/3점 모드 테스트
    - 수동: measurement hover/click/선택해제 동작 확인

## 5. 파일 변경 명세

| 경로 | 변경 유형 | 비고 |
|---|---|---|
| `webassembly/src/render/application/picked_atom_info.h` | **신규** | 값 객체 |
| `webassembly/src/render/application/render_gateway.h` | 수정 | `resolvePickedAtom` 추가 |
| `webassembly/src/render/infrastructure/vtk_render_gateway.h` | 수정 | 구현 선언 |
| `webassembly/src/render/infrastructure/vtk_render_gateway.cpp` | 수정 | VTK picker 내부 호출 |
| `webassembly/src/measurement/application/measurement_service.h` | 수정 | 서명 |
| `webassembly/src/measurement/application/measurement_service.cpp` | 수정 | 서명 |
| `webassembly/src/measurement/application/measurement_controller.cpp` | 수정 | 서명, VTK include 제거 |
| `webassembly/src/render/application/viewer_interaction_controller.cpp` | 수정 | 호출부 |
| `webassembly/cmake/modules/wb_measurement.cmake` | 수정 (검토) | VTK 링크 제거 |
| `docs/refactoring/phase19/logs/measurement_vtk_token_inventory_phase19_latest.md` | **신규** | 인벤토리 |
| `docs/refactoring/phase19/logs/dependency_gate_report_W2.md` | **신규** | 완료 리포트 |

## 6. 완료 기준 (DoD)

| 항목 | 측정 | 기준 |
|---|---|---|
| `measurement/` 하위 `vtkActor`/`vtkVolume`/`vtkSmartPointer` 참조 | grep | 0 |
| `measurement/` 하위 `#include.*vtk` | grep | 0 (`vector` 등 STL 제외) |
| `check:phase15:render-platform-isolation` | npm script | PASS |
| `check:phase19:measurement-vtk-zero` (신규 선택) | npm script | PASS |
| Phase 15의 render/platform 경계 보고서 재측정 | 회귀 보고서 | 위반 0 |
| e2e workbench-smoke (measurement 시나리오) | Playwright | PASS |

## 7. 리스크와 완화

| 리스크 | 영향 | 완화책 |
|---|---|---|
| `PickedAtomInfo`에 필요한 필드 누락 → measurement 정확도 저하 | 측정값 오류 | W0에서 현재 `AtomsTemplate::HandleMeasurementClickByPicker` 내부 소비 필드 전수조사, 값 객체에 포함 |
| picker 재실행 비용 | 성능 저하 | `resolvePickedAtom`은 이미 click 이벤트에서 단 1회만 호출되므로 실질 비용 없음 |
| W5.2 이전에 `AtomsTemplate` 서명만 바꾼 과도기 상태 | 혼란 | 커밋 메시지에 "intermediate step until W5.2" 명시 |

## 8. 산출물

- 신규 헤더 1종, 수정 파일 7종, 게이트 스크립트(선택) 1종
- `docs/refactoring/phase19/logs/measurement_vtk_token_inventory_phase19_latest.md`
- `docs/refactoring/phase19/logs/dependency_gate_report_W2.md`

## 9. Git 커밋 템플릿

```
refactor(measurement,render): phase19/W2 — drop vtkActor* from measurement API

- Introduce render::application::PickedAtomInfo
- Add RenderGateway::resolvePickedAtom(pickPos) on render side
- Change MeasurementService::handlePickerClick signature to take PickedAtomInfo
- Update viewer_interaction_controller to translate picker hit to value object
- Remove VTK includes from measurement/**

metrics:
  vtkActor references in measurement/: 1 -> 0
  render→measurement raw pointer flow: present -> none
  check:phase15:render-platform-isolation: PARTIAL -> PASS

verif:
  check:phase15:render-platform-isolation: PASS
  e2e measurement 2pt/3pt: PASS
```

---

*W2는 Phase 15 DoD의 완전 달성 상태를 확정한다. 이후 W5.2에서 `HandleMeasurementClickByPicker` 정의부 자체가 소멸하며 측정 로직은 `MeasurementService`로 귀속된다.*
