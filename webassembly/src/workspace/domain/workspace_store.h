/**
 * @file workspace_store.h
 * @brief Workspace context store that owns active selection identifiers.
 */
#pragma once

#include <cstdint>

namespace workspace {
namespace domain {

/**
 * @brief Central store for active workspace context ids.
 * @details The store only owns cross-feature context identifiers.
 *          Feature payloads remain in feature repositories.
 */
class WorkspaceStore {
public:
    /**
     * @brief Returns global workspace store instance.
     */
    static WorkspaceStore& Instance();

    /**
     * @brief Clears all active context identifiers.
     */
    void Reset();

    int32_t CurrentStructureId() const { return m_CurrentStructureId; }
    int32_t SelectedMeshId() const { return m_SelectedMeshId; }
    int32_t ActiveDensityStructureId() const { return m_ActiveDensityStructureId; }
    int32_t ActiveDensityGridMeshId() const { return m_ActiveDensityGridMeshId; }

    void SetCurrentStructureId(int32_t structureId) { m_CurrentStructureId = structureId; }
    void SetSelectedMeshId(int32_t meshId) { m_SelectedMeshId = meshId; }
    void SetActiveDensityStructureId(int32_t structureId) { m_ActiveDensityStructureId = structureId; }
    void SetActiveDensityGridMeshId(int32_t meshId) { m_ActiveDensityGridMeshId = meshId; }

private:
    WorkspaceStore() = default;

    int32_t m_CurrentStructureId { -1 };
    int32_t m_SelectedMeshId { -1 };
    int32_t m_ActiveDensityStructureId { -1 };
    int32_t m_ActiveDensityGridMeshId { -1 };
};

/**
 * @brief Convenience accessor for workspace store.
 */
WorkspaceStore& GetWorkspaceStore();

} // namespace domain
} // namespace workspace

