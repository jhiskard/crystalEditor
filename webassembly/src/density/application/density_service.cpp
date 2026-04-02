#include "density_service.h"

#include "../../atoms/atoms_template.h"

namespace density {
namespace application {

DensityService::DensityService(AtomsTemplate* atomsTemplate)
    : m_atomsTemplate(atomsTemplate) {}

bool DensityService::HasChargeDensity() const {
    return m_atomsTemplate && m_atomsTemplate->HasChargeDensity();
}

bool DensityService::IsChargeDensityVisible() const {
    return m_atomsTemplate && m_atomsTemplate->IsChargeDensityVisible();
}

void DensityService::SetChargeDensityVisible(bool visible) {
    if (!m_atomsTemplate) {
        return;
    }
    m_atomsTemplate->SetChargeDensityVisible(visible);
}

void DensityService::SyncChargeDensityViewTypeState() {
    if (!m_atomsTemplate) {
        return;
    }
    m_atomsTemplate->SyncChargeDensityViewTypeState();
}

int32_t DensityService::GetChargeDensityStructureId() const {
    if (!m_atomsTemplate) {
        return -1;
    }
    return m_atomsTemplate->GetChargeDensityStructureId();
}

void DensityService::SetChargeDensityStructureId(int32_t structureId) {
    if (!m_atomsTemplate) {
        return;
    }
    m_atomsTemplate->SetChargeDensityStructureId(structureId);
}

bool DensityService::IsAdvancedGridVisible(int32_t meshId, bool volumeMode) const {
    if (!m_atomsTemplate) {
        return false;
    }
    return m_atomsTemplate->IsChargeDensityAdvancedGridVisible(meshId, volumeMode);
}

void DensityService::SetAdvancedGridVisible(int32_t meshId, bool volumeMode, bool visible) {
    if (!m_atomsTemplate) {
        return;
    }
    m_atomsTemplate->SetChargeDensityAdvancedGridVisible(meshId, volumeMode, visible);
}

void DensityService::SetAllAdvancedGridVisible(int32_t structureId, bool volumeMode, bool visible) {
    if (!m_atomsTemplate) {
        return;
    }
    m_atomsTemplate->SetAllChargeDensityAdvancedGridVisible(structureId, volumeMode, visible);
}

void DensityService::ApplyAdvancedGridVisibilityForStructure(int32_t structureId) {
    if (!m_atomsTemplate) {
        return;
    }
    m_atomsTemplate->ApplyChargeDensityAdvancedGridVisibilityForStructure(structureId);
}

atoms::ui::ChargeDensityUI* DensityService::ChargeDensityUi() {
    if (!m_atomsTemplate) {
        return nullptr;
    }
    return m_atomsTemplate->chargeDensityUI();
}

const atoms::ui::ChargeDensityUI* DensityService::ChargeDensityUi() const {
    if (!m_atomsTemplate) {
        return nullptr;
    }
    return m_atomsTemplate->chargeDensityUI();
}

} // namespace application
} // namespace density

