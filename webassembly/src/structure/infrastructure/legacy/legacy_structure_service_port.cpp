#include "legacy_structure_service_port.h"

#include "../../../workspace/legacy/legacy_atoms_runtime.h"

namespace structure {
namespace infrastructure {
namespace legacy {

int LegacyStructureServicePort::GetStructureCount() const {
    return static_cast<int>(workspace::legacy::LegacyAtomsRuntime().GetStructures().size());
}

int32_t LegacyStructureServicePort::GetCurrentStructureId() const {
    return workspace::legacy::LegacyAtomsRuntime().GetCurrentStructureId();
}

void LegacyStructureServicePort::SetCurrentStructureId(int32_t structureId) {
    workspace::legacy::LegacyAtomsRuntime().SetCurrentStructureId(structureId);
}

std::vector<application::StructureEntryView> LegacyStructureServicePort::GetStructures() const {
    std::vector<application::StructureEntryView> output;
    const auto entries = workspace::legacy::LegacyAtomsRuntime().GetStructures();
    output.reserve(entries.size());
    for (const auto& entry : entries) {
        output.push_back({entry.id, entry.name, entry.visible});
    }
    return output;
}

bool LegacyStructureServicePort::IsStructureVisible(int32_t structureId) const {
    return workspace::legacy::LegacyAtomsRuntime().IsStructureVisible(structureId);
}

void LegacyStructureServicePort::SetStructureVisible(int32_t structureId, bool visible) {
    workspace::legacy::LegacyAtomsRuntime().SetStructureVisible(structureId, visible);
}

void LegacyStructureServicePort::RegisterStructure(int32_t structureId, const std::string& name) {
    workspace::legacy::LegacyAtomsRuntime().RegisterStructure(structureId, name);
}

void LegacyStructureServicePort::RemoveStructure(int32_t structureId) {
    workspace::legacy::LegacyAtomsRuntime().RemoveStructure(structureId);
}

void LegacyStructureServicePort::RemoveUnassignedData() {
    workspace::legacy::LegacyAtomsRuntime().RemoveUnassignedData();
}

size_t LegacyStructureServicePort::GetAtomCountForStructure(int32_t structureId) const {
    return workspace::legacy::LegacyAtomsRuntime().GetAtomCountForStructure(structureId);
}

bool LegacyStructureServicePort::IsBoundaryAtomsEnabled() const {
    return workspace::legacy::LegacyAtomsRuntime().isSurroundingsVisible();
}

void LegacyStructureServicePort::SetBoundaryAtomsEnabled(bool enabled) {
    workspace::legacy::LegacyAtomsRuntime().SetBoundaryAtomsEnabled(enabled);
}

} // namespace legacy
} // namespace infrastructure
} // namespace structure


