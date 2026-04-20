#include "density_service.h"

#include "../infrastructure/legacy/legacy_density_service_port.h"

namespace {
density::application::DensityServicePort& defaultDensityServicePort() {
    static density::infrastructure::legacy::LegacyDensityServicePort port;
    return port;
}
} // namespace

namespace density {
namespace application {

DensityService::DensityService()
    : DensityService(defaultDensityServicePort()) {}

DensityService::DensityService(DensityServicePort& port)
    : m_Port(&port) {}

bool DensityService::HasChargeDensity() const {
    return m_Port->HasChargeDensity();
}

bool DensityService::IsChargeDensityVisible() const {
    return m_Port->IsChargeDensityVisible();
}

void DensityService::SetChargeDensityVisible(bool visible) {
    m_Port->SetChargeDensityVisible(visible);
}

bool DensityService::IsSimpleViewActive() const {
    return m_Port->IsSimpleViewActive();
}

void DensityService::SyncChargeDensityViewTypeState() {
    m_Port->SyncChargeDensityViewTypeState();
}

int32_t DensityService::GetChargeDensityStructureId() const {
    return m_Port->GetChargeDensityStructureId();
}

void DensityService::SetChargeDensityStructureId(int32_t structureId) {
    m_Port->SetChargeDensityStructureId(structureId);
}

bool DensityService::IsAdvancedGridVisible(int32_t meshId, bool volumeMode) const {
    return m_Port->IsAdvancedGridVisible(meshId, volumeMode);
}

void DensityService::SetAdvancedGridVisible(int32_t meshId, bool volumeMode, bool visible) {
    m_Port->SetAdvancedGridVisible(meshId, volumeMode, visible);
}

void DensityService::SetAllAdvancedGridVisible(int32_t structureId, bool volumeMode, bool visible) {
    m_Port->SetAllAdvancedGridVisible(structureId, volumeMode, visible);
}

void DensityService::ApplyAdvancedGridVisibilityForStructure(int32_t structureId) {
    m_Port->ApplyAdvancedGridVisibilityForStructure(structureId);
}

atoms::ui::ChargeDensityUI* DensityService::ChargeDensityUi() {
    return m_Port->ChargeDensityUi();
}

const atoms::ui::ChargeDensityUI* DensityService::ChargeDensityUi() const {
    return m_Port->ChargeDensityUi();
}

} // namespace application
} // namespace density
