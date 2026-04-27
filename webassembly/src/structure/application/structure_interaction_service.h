/**
 * @file structure_interaction_service.h
 * @brief Structure atom/bond/cell interaction facade extracted from WorkspaceRuntimeModel.
 */
#pragma once

#include <cstdint>
#include <vector>

class vtkActor;

namespace structure {
namespace application {

/**
 * @brief Application facade for atom/bond/cell interaction operations.
 * @details W5.1 starts with thin-shim delegation to legacy runtime and then
 *          migrates ownership and state boundaries in later stages.
 */
class StructureInteractionService {
public:
    StructureInteractionService();

    void UpdateHoveredAtomByPicker(vtkActor* actor, double pickPos[3]);
    void ClearHover();
    void RenderAtomTooltip(float mouseX, float mouseY);

    void SelectAtomByPicker(vtkActor* actor, double pickPos[3]);
    void SelectSameElementAtomsByPicker(vtkActor* actor, double pickPos[3]);
    void ClearCreatedAtomSelection();

    bool IsAtomVisibleById(uint32_t atomId) const;
    void SetAtomVisibleById(uint32_t atomId, bool visible);
    void SetAtomVisibilityForIds(const std::vector<uint32_t>& atomIds, bool visible);

    bool IsAtomLabelVisibleById(uint32_t atomId) const;
    void SetAtomLabelVisibleById(uint32_t atomId, bool visible);
    void SetAtomLabelVisibilityForIds(const std::vector<uint32_t>& atomIds, bool visible);

    bool IsBondsVisible(int32_t structureId) const;
    bool IsBondVisibleById(uint32_t bondId) const;
    void SetBondsVisible(int32_t structureId, bool visible);
    void SetBondVisibleById(uint32_t bondId, bool visible);
    void SetBondVisibilityForIds(const std::vector<uint32_t>& bondIds, bool visible);

    bool IsBondLabelVisibleById(uint32_t bondId) const;
    void SetBondLabelVisibleById(uint32_t bondId, bool visible);
    void SetBondLabelVisibilityForIds(const std::vector<uint32_t>& bondIds, bool visible);

    bool HasUnitCell(int32_t structureId) const;
    bool IsUnitCellVisible(int32_t structureId) const;
    void SetUnitCellVisible(int32_t structureId, bool visible);
};

} // namespace application
} // namespace structure
