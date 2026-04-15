#include "density_service_legacy_port_adapter.h"

#include "../legacy/atoms_template_facade.h"

namespace atoms {
namespace application {

DensityServiceLegacyPortAdapter::DensityServiceLegacyPortAdapter(AtomsTemplate* atomsTemplate)
    : m_atomsTemplate(atomsTemplate) {}

bool DensityServiceLegacyPortAdapter::HasChargeDensity() const {
    if (!m_atomsTemplate) {
        return false;
    }
    return m_atomsTemplate->HasChargeDensity();
}

bool DensityServiceLegacyPortAdapter::IsChargeDensityVisible() const {
    if (!m_atomsTemplate) {
        return false;
    }
    return m_atomsTemplate->IsChargeDensityVisible();
}

void DensityServiceLegacyPortAdapter::SetChargeDensityVisible(bool visible) {
    if (!m_atomsTemplate) {
        return;
    }
    m_atomsTemplate->SetChargeDensityVisible(visible);
}

bool DensityServiceLegacyPortAdapter::IsSimpleViewActive() const {
    if (!m_atomsTemplate) {
        return false;
    }
    return m_atomsTemplate->IsChargeDensitySimpleViewActive();
}

void DensityServiceLegacyPortAdapter::SyncChargeDensityViewTypeState() {
    if (!m_atomsTemplate) {
        return;
    }
    m_atomsTemplate->SyncChargeDensityViewTypeState();
}

int32_t DensityServiceLegacyPortAdapter::GetChargeDensityStructureId() const {
    if (!m_atomsTemplate) {
        return -1;
    }
    return m_atomsTemplate->GetChargeDensityStructureId();
}

void DensityServiceLegacyPortAdapter::SetChargeDensityStructureId(int32_t structureId) {
    if (!m_atomsTemplate) {
        return;
    }
    m_atomsTemplate->SetChargeDensityStructureId(structureId);
}

bool DensityServiceLegacyPortAdapter::IsAdvancedGridVisible(int32_t meshId, bool volumeMode) const {
    if (!m_atomsTemplate) {
        return false;
    }
    return m_atomsTemplate->IsChargeDensityAdvancedGridVisible(meshId, volumeMode);
}

void DensityServiceLegacyPortAdapter::SetAdvancedGridVisible(int32_t meshId, bool volumeMode, bool visible) {
    if (!m_atomsTemplate) {
        return;
    }
    m_atomsTemplate->SetChargeDensityAdvancedGridVisible(meshId, volumeMode, visible);
}

void DensityServiceLegacyPortAdapter::SetAllAdvancedGridVisible(
    int32_t structureId,
    bool volumeMode,
    bool visible) {
    if (!m_atomsTemplate) {
        return;
    }
    m_atomsTemplate->SetAllChargeDensityAdvancedGridVisible(structureId, volumeMode, visible);
}

void DensityServiceLegacyPortAdapter::ApplyAdvancedGridVisibilityForStructure(int32_t structureId) {
    if (!m_atomsTemplate) {
        return;
    }
    m_atomsTemplate->ApplyChargeDensityAdvancedGridVisibilityForStructure(structureId);
}

atoms::ui::ChargeDensityUI* DensityServiceLegacyPortAdapter::ChargeDensityUi() {
    if (!m_atomsTemplate) {
        return nullptr;
    }
    return m_atomsTemplate->chargeDensityUI();
}

const atoms::ui::ChargeDensityUI* DensityServiceLegacyPortAdapter::ChargeDensityUi() const {
    if (!m_atomsTemplate) {
        return nullptr;
    }
    return m_atomsTemplate->chargeDensityUI();
}

} // namespace application
} // namespace atoms

