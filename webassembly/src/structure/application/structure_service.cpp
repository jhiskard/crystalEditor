#include "structure_service.h"

#include "../../workspace/legacy/atoms_template_facade.h"

namespace structure {
namespace application {

int StructureService::GetStructureCount() const {
    return static_cast<int>(AtomsTemplate::Instance().GetStructures().size());
}

int32_t StructureService::GetCurrentStructureId() const {
    return AtomsTemplate::Instance().GetCurrentStructureId();
}

void StructureService::SetCurrentStructureId(int32_t structureId) {
    AtomsTemplate::Instance().SetCurrentStructureId(structureId);
}

std::vector<StructureEntryView> StructureService::GetStructures() const {
    std::vector<StructureEntryView> output;
    const auto entries = AtomsTemplate::Instance().GetStructures();
    output.reserve(entries.size());
    for (const auto& entry : entries) {
        output.push_back({entry.id, entry.name, entry.visible});
    }
    return output;
}

bool StructureService::IsStructureVisible(int32_t structureId) const {
    return AtomsTemplate::Instance().IsStructureVisible(structureId);
}

void StructureService::SetStructureVisible(int32_t structureId, bool visible) {
    AtomsTemplate::Instance().SetStructureVisible(structureId, visible);
}

void StructureService::RegisterStructure(int32_t structureId, const std::string& name) {
    AtomsTemplate::Instance().RegisterStructure(structureId, name);
}

void StructureService::RemoveStructure(int32_t structureId) {
    AtomsTemplate::Instance().RemoveStructure(structureId);
}

void StructureService::RemoveUnassignedData() {
    AtomsTemplate::Instance().RemoveUnassignedData();
}

size_t StructureService::GetAtomCountForStructure(int32_t structureId) const {
    return AtomsTemplate::Instance().GetAtomCountForStructure(structureId);
}

bool StructureService::IsBoundaryAtomsEnabled() const {
    return AtomsTemplate::Instance().isSurroundingsVisible();
}

void StructureService::SetBoundaryAtomsEnabled(bool enabled) {
    AtomsTemplate::Instance().SetBoundaryAtomsEnabled(enabled);
}

} // namespace application
} // namespace structure


