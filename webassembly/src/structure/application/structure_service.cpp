#include "structure_service.h"

#include "../infrastructure/legacy/legacy_structure_service_port.h"

namespace {
structure::application::StructureServicePort& defaultStructureServicePort() {
    static structure::infrastructure::legacy::LegacyStructureServicePort port;
    return port;
}
} // namespace

namespace structure {
namespace application {

StructureService::StructureService()
    : StructureService(defaultStructureServicePort()) {}

StructureService::StructureService(StructureServicePort& port)
    : m_Port(&port) {}

int StructureService::GetStructureCount() const {
    return m_Port->GetStructureCount();
}

int32_t StructureService::GetCurrentStructureId() const {
    return m_Port->GetCurrentStructureId();
}

void StructureService::SetCurrentStructureId(int32_t structureId) {
    m_Port->SetCurrentStructureId(structureId);
}

std::vector<StructureEntryView> StructureService::GetStructures() const {
    return m_Port->GetStructures();
}

bool StructureService::IsStructureVisible(int32_t structureId) const {
    return m_Port->IsStructureVisible(structureId);
}

void StructureService::SetStructureVisible(int32_t structureId, bool visible) {
    m_Port->SetStructureVisible(structureId, visible);
}

void StructureService::RegisterStructure(int32_t structureId, const std::string& name) {
    m_Port->RegisterStructure(structureId, name);
}

void StructureService::RemoveStructure(int32_t structureId) {
    m_Port->RemoveStructure(structureId);
}

void StructureService::RemoveUnassignedData() {
    m_Port->RemoveUnassignedData();
}

size_t StructureService::GetAtomCountForStructure(int32_t structureId) const {
    return m_Port->GetAtomCountForStructure(structureId);
}

bool StructureService::IsBoundaryAtomsEnabled() const {
    return m_Port->IsBoundaryAtomsEnabled();
}

void StructureService::SetBoundaryAtomsEnabled(bool enabled) {
    m_Port->SetBoundaryAtomsEnabled(enabled);
}

} // namespace application
} // namespace structure
