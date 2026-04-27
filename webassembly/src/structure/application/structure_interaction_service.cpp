#include "structure_interaction_service.h"

#include "../../workspace/runtime/legacy_atoms_runtime.h"

namespace structure {
namespace application {

StructureInteractionService::StructureInteractionService() = default;

void StructureInteractionService::UpdateHoveredAtomByPicker(vtkActor* actor, double pickPos[3]) {
    workspace::legacy::WorkspaceRuntimeModelRef().UpdateHoveredAtomByPicker(actor, pickPos);
}

void StructureInteractionService::ClearHover() {
    workspace::legacy::WorkspaceRuntimeModelRef().ClearHover();
}

void StructureInteractionService::RenderAtomTooltip(float mouseX, float mouseY) {
    workspace::legacy::WorkspaceRuntimeModelRef().RenderAtomTooltip(mouseX, mouseY);
}

void StructureInteractionService::SelectAtomByPicker(vtkActor* actor, double pickPos[3]) {
    workspace::legacy::WorkspaceRuntimeModelRef().SelectAtomByPicker(actor, pickPos);
}

void StructureInteractionService::SelectSameElementAtomsByPicker(vtkActor* actor, double pickPos[3]) {
    workspace::legacy::WorkspaceRuntimeModelRef().SelectSameElementAtomsByPicker(actor, pickPos);
}

void StructureInteractionService::ClearCreatedAtomSelection() {
    workspace::legacy::WorkspaceRuntimeModelRef().ClearCreatedAtomSelection();
}

bool StructureInteractionService::IsAtomVisibleById(uint32_t atomId) const {
    return workspace::legacy::WorkspaceRuntimeModelRef().IsAtomVisibleById(atomId);
}

void StructureInteractionService::SetAtomVisibleById(uint32_t atomId, bool visible) {
    workspace::legacy::WorkspaceRuntimeModelRef().SetAtomVisibleById(atomId, visible);
}

void StructureInteractionService::SetAtomVisibilityForIds(
    const std::vector<uint32_t>& atomIds,
    bool visible) {
    workspace::legacy::WorkspaceRuntimeModelRef().SetAtomVisibilityForIds(atomIds, visible);
}

bool StructureInteractionService::IsAtomLabelVisibleById(uint32_t atomId) const {
    return workspace::legacy::WorkspaceRuntimeModelRef().IsAtomLabelVisibleById(atomId);
}

void StructureInteractionService::SetAtomLabelVisibleById(uint32_t atomId, bool visible) {
    workspace::legacy::WorkspaceRuntimeModelRef().SetAtomLabelVisibleById(atomId, visible);
}

void StructureInteractionService::SetAtomLabelVisibilityForIds(
    const std::vector<uint32_t>& atomIds,
    bool visible) {
    workspace::legacy::WorkspaceRuntimeModelRef().SetAtomLabelVisibilityForIds(atomIds, visible);
}

bool StructureInteractionService::IsBondsVisible(int32_t structureId) const {
    return workspace::legacy::WorkspaceRuntimeModelRef().IsBondsVisible(structureId);
}

bool StructureInteractionService::IsBondVisibleById(uint32_t bondId) const {
    return workspace::legacy::WorkspaceRuntimeModelRef().IsBondVisibleById(bondId);
}

void StructureInteractionService::SetBondsVisible(int32_t structureId, bool visible) {
    workspace::legacy::WorkspaceRuntimeModelRef().SetBondsVisible(structureId, visible);
}

void StructureInteractionService::SetBondVisibleById(uint32_t bondId, bool visible) {
    workspace::legacy::WorkspaceRuntimeModelRef().SetBondVisibleById(bondId, visible);
}

void StructureInteractionService::SetBondVisibilityForIds(
    const std::vector<uint32_t>& bondIds,
    bool visible) {
    workspace::legacy::WorkspaceRuntimeModelRef().SetBondVisibilityForIds(bondIds, visible);
}

bool StructureInteractionService::IsBondLabelVisibleById(uint32_t bondId) const {
    return workspace::legacy::WorkspaceRuntimeModelRef().IsBondLabelVisibleById(bondId);
}

void StructureInteractionService::SetBondLabelVisibleById(uint32_t bondId, bool visible) {
    workspace::legacy::WorkspaceRuntimeModelRef().SetBondLabelVisibleById(bondId, visible);
}

void StructureInteractionService::SetBondLabelVisibilityForIds(
    const std::vector<uint32_t>& bondIds,
    bool visible) {
    workspace::legacy::WorkspaceRuntimeModelRef().SetBondLabelVisibilityForIds(bondIds, visible);
}

bool StructureInteractionService::HasUnitCell(int32_t structureId) const {
    return workspace::legacy::WorkspaceRuntimeModelRef().HasUnitCell(structureId);
}

bool StructureInteractionService::IsUnitCellVisible(int32_t structureId) const {
    return workspace::legacy::WorkspaceRuntimeModelRef().IsUnitCellVisible(structureId);
}

void StructureInteractionService::SetUnitCellVisible(int32_t structureId, bool visible) {
    workspace::legacy::WorkspaceRuntimeModelRef().SetUnitCellVisible(structureId, visible);
}

} // namespace application
} // namespace structure

