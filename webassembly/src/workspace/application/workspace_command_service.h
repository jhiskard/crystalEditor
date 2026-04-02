/**
 * @file workspace_command_service.h
 * @brief Command service for workspace context mutations.
 */
#pragma once

#include <cstdint>

namespace workspace {
namespace application {

/**
 * @brief Write facade for active workspace context ids.
 * @details Mutation entrypoints are centralized here so callers do not
 *          modify the domain store directly.
 */
class WorkspaceCommandService {
public:
    /**
     * @brief Returns singleton command service instance.
     */
    static WorkspaceCommandService& Instance();

    void Reset();
    void SetCurrentStructureId(int32_t structureId);
    void SetSelectedMeshId(int32_t meshId);
    void SetActiveDensityStructureId(int32_t structureId);
    void SetActiveDensityGridMeshId(int32_t meshId);

private:
    WorkspaceCommandService() = default;
};

/**
 * @brief Convenience accessor for workspace command service.
 */
WorkspaceCommandService& GetWorkspaceCommandService();

} // namespace application
} // namespace workspace

