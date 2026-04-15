#include "structure_service.h"

namespace structure {
namespace application {

StructureService::StructureService(StructureServicePort* port)
    : m_port(port) {}

int StructureService::GetStructureCount() const {
    if (!m_port) {
        return 0;
    }
    return m_port->GetStructureCount();
}

int32_t StructureService::GetCurrentStructureId() const {
    if (!m_port) {
        return -1;
    }
    return m_port->GetCurrentStructureId();
}

void StructureService::SetCurrentStructureId(int32_t structureId) {
    if (!m_port) {
        return;
    }
    m_port->SetCurrentStructureId(structureId);
}

std::vector<StructureEntryView> StructureService::GetStructures() const {
    if (!m_port) {
        return {};
    }
    return m_port->GetStructures();
}

bool StructureService::IsStructureVisible(int32_t structureId) const {
    if (!m_port) {
        return false;
    }
    return m_port->IsStructureVisible(structureId);
}

void StructureService::SetStructureVisible(int32_t structureId, bool visible) {
    if (!m_port) {
        return;
    }
    m_port->SetStructureVisible(structureId, visible);
}

void StructureService::RegisterStructure(int32_t structureId, const std::string& name) {
    if (!m_port) {
        return;
    }
    m_port->RegisterStructure(structureId, name);
}

void StructureService::RemoveStructure(int32_t structureId) {
    if (!m_port) {
        return;
    }
    m_port->RemoveStructure(structureId);
}

void StructureService::RemoveUnassignedData() {
    if (!m_port) {
        return;
    }
    m_port->RemoveUnassignedData();
}

size_t StructureService::GetAtomCountForStructure(int32_t structureId) const {
    if (!m_port) {
        return 0;
    }
    return m_port->GetAtomCountForStructure(structureId);
}

bool StructureService::IsBoundaryAtomsEnabled() const {
    if (!m_port) {
        return false;
    }
    return m_port->IsBoundaryAtomsEnabled();
}

void StructureService::SetBoundaryAtomsEnabled(bool enabled) {
    if (!m_port) {
        return;
    }
    m_port->SetBoundaryAtomsEnabled(enabled);
}

} // namespace application
} // namespace structure

