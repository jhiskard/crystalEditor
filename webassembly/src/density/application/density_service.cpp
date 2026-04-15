#include "density_service.h"

namespace density {
namespace application {

DensityService::DensityService(DensityServicePort* port)
    : m_port(port) {}

bool DensityService::HasChargeDensity() const {
    if (!m_port) {
        return false;
    }
    return m_port->HasChargeDensity();
}

bool DensityService::IsChargeDensityVisible() const {
    if (!m_port) {
        return false;
    }
    return m_port->IsChargeDensityVisible();
}

void DensityService::SetChargeDensityVisible(bool visible) {
    if (!m_port) {
        return;
    }
    m_port->SetChargeDensityVisible(visible);
}

bool DensityService::IsSimpleViewActive() const {
    if (!m_port) {
        return false;
    }
    return m_port->IsSimpleViewActive();
}

void DensityService::SyncChargeDensityViewTypeState() {
    if (!m_port) {
        return;
    }
    m_port->SyncChargeDensityViewTypeState();
}

int32_t DensityService::GetChargeDensityStructureId() const {
    if (!m_port) {
        return -1;
    }
    return m_port->GetChargeDensityStructureId();
}

void DensityService::SetChargeDensityStructureId(int32_t structureId) {
    if (!m_port) {
        return;
    }
    m_port->SetChargeDensityStructureId(structureId);
}

bool DensityService::IsAdvancedGridVisible(int32_t meshId, bool volumeMode) const {
    if (!m_port) {
        return false;
    }
    return m_port->IsAdvancedGridVisible(meshId, volumeMode);
}

void DensityService::SetAdvancedGridVisible(int32_t meshId, bool volumeMode, bool visible) {
    if (!m_port) {
        return;
    }
    m_port->SetAdvancedGridVisible(meshId, volumeMode, visible);
}

void DensityService::SetAllAdvancedGridVisible(int32_t structureId, bool volumeMode, bool visible) {
    if (!m_port) {
        return;
    }
    m_port->SetAllAdvancedGridVisible(structureId, volumeMode, visible);
}

void DensityService::ApplyAdvancedGridVisibilityForStructure(int32_t structureId) {
    if (!m_port) {
        return;
    }
    m_port->ApplyAdvancedGridVisibilityForStructure(structureId);
}

atoms::ui::ChargeDensityUI* DensityService::ChargeDensityUi() {
    if (!m_port) {
        return nullptr;
    }
    return m_port->ChargeDensityUi();
}

const atoms::ui::ChargeDensityUI* DensityService::ChargeDensityUi() const {
    if (!m_port) {
        return nullptr;
    }
    return m_port->ChargeDensityUi();
}

} // namespace application
} // namespace density

