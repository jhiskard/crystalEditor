/**
 * @file workspace_query_service.h
 * @brief Read-only workspace context query service.
 */
#pragma once

#include <cstdint>

namespace workspace {
namespace application {

/**
 * @brief Read facade for active workspace context ids.
 * @details This service exposes query-only access so presenters can avoid
 *          mutating shared context state directly.
 */
class WorkspaceQueryService {
public:
    /**
     * @brief Returns singleton query service instance.
     */
    static WorkspaceQueryService& Instance();

    int32_t CurrentStructureId() const;
    int32_t SelectedMeshId() const;
    int32_t ActiveDensityStructureId() const;
    int32_t ActiveDensityGridMeshId() const;

private:
    WorkspaceQueryService() = default;
};

/**
 * @brief Convenience accessor for workspace query service.
 */
WorkspaceQueryService& GetWorkspaceQueryService();

} // namespace application
} // namespace workspace

