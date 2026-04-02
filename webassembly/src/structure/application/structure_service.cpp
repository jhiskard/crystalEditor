#include "structure_service.h"

#include "../../atoms/atoms_template.h"

namespace structure {
namespace application {

StructureService::StructureService(AtomsTemplate* atomsTemplate)
    : m_atomsTemplate(atomsTemplate) {}

int StructureService::GetStructureCount() const {
    if (!m_atomsTemplate) {
        return 0;
    }
    return static_cast<int>(m_atomsTemplate->GetStructures().size());
}

int32_t StructureService::GetCurrentStructureId() const {
    if (!m_atomsTemplate) {
        return -1;
    }
    return m_atomsTemplate->GetCurrentStructureId();
}

void StructureService::SetCurrentStructureId(int32_t structureId) {
    if (!m_atomsTemplate) {
        return;
    }
    m_atomsTemplate->SetCurrentStructureId(structureId);
}

std::vector<StructureEntryView> StructureService::GetStructures() const {
    std::vector<StructureEntryView> output;
    if (!m_atomsTemplate) {
        return output;
    }

    const auto entries = m_atomsTemplate->GetStructures();
    output.reserve(entries.size());
    for (const auto& entry : entries) {
        output.push_back(StructureEntryView { entry.id, entry.name, entry.visible });
    }
    return output;
}

bool StructureService::IsStructureVisible(int32_t structureId) const {
    if (!m_atomsTemplate) {
        return false;
    }
    return m_atomsTemplate->IsStructureVisible(structureId);
}

void StructureService::SetStructureVisible(int32_t structureId, bool visible) {
    if (!m_atomsTemplate) {
        return;
    }
    m_atomsTemplate->SetStructureVisible(structureId, visible);
}

void StructureService::RegisterStructure(int32_t structureId, const std::string& name) {
    if (!m_atomsTemplate) {
        return;
    }
    m_atomsTemplate->RegisterStructure(structureId, name);
}

void StructureService::RemoveStructure(int32_t structureId) {
    if (!m_atomsTemplate) {
        return;
    }
    m_atomsTemplate->RemoveStructure(structureId);
}

void StructureService::RemoveUnassignedData() {
    if (!m_atomsTemplate) {
        return;
    }
    m_atomsTemplate->RemoveUnassignedData();
}

size_t StructureService::GetAtomCountForStructure(int32_t structureId) const {
    if (!m_atomsTemplate) {
        return 0;
    }
    return m_atomsTemplate->GetAtomCountForStructure(structureId);
}

} // namespace application
} // namespace structure

