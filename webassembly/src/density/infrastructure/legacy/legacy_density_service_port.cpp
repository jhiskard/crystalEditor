#include "legacy_density_service_port.h"

#include "../../../workspace/legacy/legacy_atoms_runtime.h"

namespace density {
namespace infrastructure {
namespace legacy {

bool LegacyDensityServicePort::HasChargeDensity() const {
    return workspace::legacy::LegacyAtomsRuntime().HasChargeDensity();
}

bool LegacyDensityServicePort::IsChargeDensityVisible() const {
    return workspace::legacy::LegacyAtomsRuntime().IsChargeDensityVisible();
}

void LegacyDensityServicePort::SetChargeDensityVisible(bool visible) {
    workspace::legacy::LegacyAtomsRuntime().SetChargeDensityVisible(visible);
}

bool LegacyDensityServicePort::IsSimpleViewActive() const {
    return workspace::legacy::LegacyAtomsRuntime().IsChargeDensitySimpleViewActive();
}

void LegacyDensityServicePort::SyncChargeDensityViewTypeState() {
    workspace::legacy::LegacyAtomsRuntime().SyncChargeDensityViewTypeState();
}

int32_t LegacyDensityServicePort::GetChargeDensityStructureId() const {
    return workspace::legacy::LegacyAtomsRuntime().GetChargeDensityStructureId();
}

void LegacyDensityServicePort::SetChargeDensityStructureId(int32_t structureId) {
    workspace::legacy::LegacyAtomsRuntime().SetChargeDensityStructureId(structureId);
}

bool LegacyDensityServicePort::IsAdvancedGridVisible(int32_t meshId, bool volumeMode) const {
    return workspace::legacy::LegacyAtomsRuntime().IsChargeDensityAdvancedGridVisible(meshId, volumeMode);
}

void LegacyDensityServicePort::SetAdvancedGridVisible(int32_t meshId, bool volumeMode, bool visible) {
    workspace::legacy::LegacyAtomsRuntime().SetChargeDensityAdvancedGridVisible(meshId, volumeMode, visible);
}

void LegacyDensityServicePort::SetAllAdvancedGridVisible(int32_t structureId, bool volumeMode, bool visible) {
    workspace::legacy::LegacyAtomsRuntime().SetAllChargeDensityAdvancedGridVisible(structureId, volumeMode, visible);
}

void LegacyDensityServicePort::ApplyAdvancedGridVisibilityForStructure(int32_t structureId) {
    workspace::legacy::LegacyAtomsRuntime().ApplyChargeDensityAdvancedGridVisibilityForStructure(structureId);
}

atoms::ui::ChargeDensityUI* LegacyDensityServicePort::ChargeDensityUi() {
    return workspace::legacy::LegacyAtomsRuntime().chargeDensityUI();
}

const atoms::ui::ChargeDensityUI* LegacyDensityServicePort::ChargeDensityUi() const {
    return workspace::legacy::LegacyAtomsRuntime().chargeDensityUI();
}

} // namespace legacy
} // namespace infrastructure
} // namespace density


