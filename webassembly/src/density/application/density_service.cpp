#include "density_service.h"

#include "../../workspace/legacy/atoms_template_facade.h"

namespace density {
namespace application {

bool DensityService::HasChargeDensity() const {
    return AtomsTemplate::Instance().HasChargeDensity();
}

bool DensityService::IsChargeDensityVisible() const {
    return AtomsTemplate::Instance().IsChargeDensityVisible();
}

void DensityService::SetChargeDensityVisible(bool visible) {
    AtomsTemplate::Instance().SetChargeDensityVisible(visible);
}

bool DensityService::IsSimpleViewActive() const {
    return AtomsTemplate::Instance().IsChargeDensitySimpleViewActive();
}

void DensityService::SyncChargeDensityViewTypeState() {
    AtomsTemplate::Instance().SyncChargeDensityViewTypeState();
}

int32_t DensityService::GetChargeDensityStructureId() const {
    return AtomsTemplate::Instance().GetChargeDensityStructureId();
}

void DensityService::SetChargeDensityStructureId(int32_t structureId) {
    AtomsTemplate::Instance().SetChargeDensityStructureId(structureId);
}

bool DensityService::IsAdvancedGridVisible(int32_t meshId, bool volumeMode) const {
    return AtomsTemplate::Instance().IsChargeDensityAdvancedGridVisible(meshId, volumeMode);
}

void DensityService::SetAdvancedGridVisible(int32_t meshId, bool volumeMode, bool visible) {
    AtomsTemplate::Instance().SetChargeDensityAdvancedGridVisible(meshId, volumeMode, visible);
}

void DensityService::SetAllAdvancedGridVisible(int32_t structureId, bool volumeMode, bool visible) {
    AtomsTemplate::Instance().SetAllChargeDensityAdvancedGridVisible(structureId, volumeMode, visible);
}

void DensityService::ApplyAdvancedGridVisibilityForStructure(int32_t structureId) {
    AtomsTemplate::Instance().ApplyChargeDensityAdvancedGridVisibilityForStructure(structureId);
}

atoms::ui::ChargeDensityUI* DensityService::ChargeDensityUi() {
    return AtomsTemplate::Instance().chargeDensityUI();
}

const atoms::ui::ChargeDensityUI* DensityService::ChargeDensityUi() const {
    return AtomsTemplate::Instance().chargeDensityUI();
}

} // namespace application
} // namespace density
