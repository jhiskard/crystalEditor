#include "structure_interaction_service.h"

#include "../../workspace/legacy/legacy_atoms_runtime.h"

namespace structure {
namespace application {

StructureInteractionService::StructureInteractionService() = default;

void StructureInteractionService::UpdateHoveredAtomByPicker(vtkActor* actor, double pickPos[3]) {
    workspace::legacy::LegacyAtomsRuntime().UpdateHoveredAtomByPicker(actor, pickPos);
}

void StructureInteractionService::ClearHover() {
    workspace::legacy::LegacyAtomsRuntime().ClearHover();
}

void StructureInteractionService::RenderAtomTooltip(float mouseX, float mouseY) {
    workspace::legacy::LegacyAtomsRuntime().RenderAtomTooltip(mouseX, mouseY);
}

void StructureInteractionService::SelectAtomByPicker(vtkActor* actor, double pickPos[3]) {
    workspace::legacy::LegacyAtomsRuntime().SelectAtomByPicker(actor, pickPos);
}

void StructureInteractionService::SelectSameElementAtomsByPicker(vtkActor* actor, double pickPos[3]) {
    workspace::legacy::LegacyAtomsRuntime().SelectSameElementAtomsByPicker(actor, pickPos);
}

void StructureInteractionService::ClearCreatedAtomSelection() {
    workspace::legacy::LegacyAtomsRuntime().ClearCreatedAtomSelection();
}

bool StructureInteractionService::IsAtomVisibleById(uint32_t atomId) const {
    return workspace::legacy::LegacyAtomsRuntime().IsAtomVisibleById(atomId);
}

void StructureInteractionService::SetAtomVisibleById(uint32_t atomId, bool visible) {
    workspace::legacy::LegacyAtomsRuntime().SetAtomVisibleById(atomId, visible);
}

void StructureInteractionService::SetAtomVisibilityForIds(
    const std::vector<uint32_t>& atomIds,
    bool visible) {
    workspace::legacy::LegacyAtomsRuntime().SetAtomVisibilityForIds(atomIds, visible);
}

bool StructureInteractionService::IsAtomLabelVisibleById(uint32_t atomId) const {
    return workspace::legacy::LegacyAtomsRuntime().IsAtomLabelVisibleById(atomId);
}

void StructureInteractionService::SetAtomLabelVisibleById(uint32_t atomId, bool visible) {
    workspace::legacy::LegacyAtomsRuntime().SetAtomLabelVisibleById(atomId, visible);
}

void StructureInteractionService::SetAtomLabelVisibilityForIds(
    const std::vector<uint32_t>& atomIds,
    bool visible) {
    workspace::legacy::LegacyAtomsRuntime().SetAtomLabelVisibilityForIds(atomIds, visible);
}

bool StructureInteractionService::IsBondsVisible(int32_t structureId) const {
    return workspace::legacy::LegacyAtomsRuntime().IsBondsVisible(structureId);
}

bool StructureInteractionService::IsBondVisibleById(uint32_t bondId) const {
    return workspace::legacy::LegacyAtomsRuntime().IsBondVisibleById(bondId);
}

void StructureInteractionService::SetBondsVisible(int32_t structureId, bool visible) {
    workspace::legacy::LegacyAtomsRuntime().SetBondsVisible(structureId, visible);
}

void StructureInteractionService::SetBondVisibleById(uint32_t bondId, bool visible) {
    workspace::legacy::LegacyAtomsRuntime().SetBondVisibleById(bondId, visible);
}

void StructureInteractionService::SetBondVisibilityForIds(
    const std::vector<uint32_t>& bondIds,
    bool visible) {
    workspace::legacy::LegacyAtomsRuntime().SetBondVisibilityForIds(bondIds, visible);
}

bool StructureInteractionService::IsBondLabelVisibleById(uint32_t bondId) const {
    return workspace::legacy::LegacyAtomsRuntime().IsBondLabelVisibleById(bondId);
}

void StructureInteractionService::SetBondLabelVisibleById(uint32_t bondId, bool visible) {
    workspace::legacy::LegacyAtomsRuntime().SetBondLabelVisibleById(bondId, visible);
}

void StructureInteractionService::SetBondLabelVisibilityForIds(
    const std::vector<uint32_t>& bondIds,
    bool visible) {
    workspace::legacy::LegacyAtomsRuntime().SetBondLabelVisibilityForIds(bondIds, visible);
}

bool StructureInteractionService::HasUnitCell(int32_t structureId) const {
    return workspace::legacy::LegacyAtomsRuntime().HasUnitCell(structureId);
}

bool StructureInteractionService::IsUnitCellVisible(int32_t structureId) const {
    return workspace::legacy::LegacyAtomsRuntime().IsUnitCellVisible(structureId);
}

void StructureInteractionService::SetUnitCellVisible(int32_t structureId, bool visible) {
    workspace::legacy::LegacyAtomsRuntime().SetUnitCellVisible(structureId, visible);
}

} // namespace application
} // namespace structure

