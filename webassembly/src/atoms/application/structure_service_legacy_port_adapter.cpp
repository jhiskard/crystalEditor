#include "structure_service_legacy_port_adapter.h"

#include "../legacy/atoms_template_facade.h"

namespace atoms {
namespace application {

StructureServiceLegacyPortAdapter::StructureServiceLegacyPortAdapter(AtomsTemplate* atomsTemplate)
    : m_atomsTemplate(atomsTemplate) {}

int StructureServiceLegacyPortAdapter::GetStructureCount() const {
    if (!m_atomsTemplate) {
        return 0;
    }
    return static_cast<int>(m_atomsTemplate->GetStructures().size());
}

int32_t StructureServiceLegacyPortAdapter::GetCurrentStructureId() const {
    if (!m_atomsTemplate) {
        return -1;
    }
    return m_atomsTemplate->GetCurrentStructureId();
}

void StructureServiceLegacyPortAdapter::SetCurrentStructureId(int32_t structureId) {
    if (!m_atomsTemplate) {
        return;
    }
    m_atomsTemplate->SetCurrentStructureId(structureId);
}

std::vector<structure::application::StructureEntryView>
StructureServiceLegacyPortAdapter::GetStructures() const {
    std::vector<structure::application::StructureEntryView> output;
    if (!m_atomsTemplate) {
        return output;
    }

    const auto entries = m_atomsTemplate->GetStructures();
    output.reserve(entries.size());
    for (const auto& entry : entries) {
        output.push_back({ entry.id, entry.name, entry.visible });
    }
    return output;
}

bool StructureServiceLegacyPortAdapter::IsStructureVisible(int32_t structureId) const {
    if (!m_atomsTemplate) {
        return false;
    }
    return m_atomsTemplate->IsStructureVisible(structureId);
}

void StructureServiceLegacyPortAdapter::SetStructureVisible(int32_t structureId, bool visible) {
    if (!m_atomsTemplate) {
        return;
    }
    m_atomsTemplate->SetStructureVisible(structureId, visible);
}

void StructureServiceLegacyPortAdapter::RegisterStructure(int32_t structureId, const std::string& name) {
    if (!m_atomsTemplate) {
        return;
    }
    m_atomsTemplate->RegisterStructure(structureId, name);
}

void StructureServiceLegacyPortAdapter::RemoveStructure(int32_t structureId) {
    if (!m_atomsTemplate) {
        return;
    }
    m_atomsTemplate->RemoveStructure(structureId);
}

void StructureServiceLegacyPortAdapter::RemoveUnassignedData() {
    if (!m_atomsTemplate) {
        return;
    }
    m_atomsTemplate->RemoveUnassignedData();
}

size_t StructureServiceLegacyPortAdapter::GetAtomCountForStructure(int32_t structureId) const {
    if (!m_atomsTemplate) {
        return 0;
    }
    return m_atomsTemplate->GetAtomCountForStructure(structureId);
}

bool StructureServiceLegacyPortAdapter::IsBoundaryAtomsEnabled() const {
    if (!m_atomsTemplate) {
        return false;
    }
    return m_atomsTemplate->isSurroundingsVisible();
}

void StructureServiceLegacyPortAdapter::SetBoundaryAtomsEnabled(bool enabled) {
    if (!m_atomsTemplate) {
        return;
    }
    m_atomsTemplate->SetBoundaryAtomsEnabled(enabled);
}

} // namespace application
} // namespace atoms
