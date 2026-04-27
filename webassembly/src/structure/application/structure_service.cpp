#include "structure_service.h"

#include "structure_lifecycle_service.h"

#include "../../workspace/legacy/legacy_atoms_runtime.h"

namespace structure {
namespace application {

StructureService::StructureService() = default;

int StructureService::GetStructureCount() const {
    return static_cast<int>(workspace::legacy::LegacyAtomsRuntime().GetStructures().size());
}

int32_t StructureService::GetCurrentStructureId() const {
    return workspace::legacy::LegacyAtomsRuntime().GetCurrentStructureId();
}

void StructureService::SetCurrentStructureId(int32_t structureId) {
    workspace::legacy::LegacyAtomsRuntime().SetCurrentStructureId(structureId);
}

std::vector<StructureEntryView> StructureService::GetStructures() const {
    std::vector<StructureEntryView> output;
    const auto entries = workspace::legacy::LegacyAtomsRuntime().GetStructures();
    output.reserve(entries.size());
    for (const auto& entry : entries) {
        output.push_back({ entry.id, entry.name, entry.visible });
    }
    return output;
}

bool StructureService::IsStructureVisible(int32_t structureId) const {
    return workspace::legacy::LegacyAtomsRuntime().IsStructureVisible(structureId);
}

void StructureService::SetStructureVisible(int32_t structureId, bool visible) {
    workspace::legacy::LegacyAtomsRuntime().SetStructureVisible(structureId, visible);
}

void StructureService::RegisterStructure(int32_t structureId, const std::string& name) {
    static StructureLifecycleService lifecycle;
    lifecycle.RegisterStructure(structureId, name);
}

void StructureService::RemoveStructure(int32_t structureId) {
    static StructureLifecycleService lifecycle;
    lifecycle.RemoveStructure(structureId);
}

void StructureService::RemoveUnassignedData() {
    static StructureLifecycleService lifecycle;
    lifecycle.RemoveUnassignedData();
}

size_t StructureService::GetAtomCountForStructure(int32_t structureId) const {
    return workspace::legacy::LegacyAtomsRuntime().GetAtomCountForStructure(structureId);
}

bool StructureService::IsBoundaryAtomsEnabled() const {
    return workspace::legacy::LegacyAtomsRuntime().isSurroundingsVisible();
}

void StructureService::SetBoundaryAtomsEnabled(bool enabled) {
    workspace::legacy::LegacyAtomsRuntime().SetBoundaryAtomsEnabled(enabled);
}

} // namespace application
} // namespace structure



