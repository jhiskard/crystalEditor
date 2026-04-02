#include "workspace_query_service.h"

#include "../domain/workspace_store.h"

namespace workspace {
namespace application {

WorkspaceQueryService& WorkspaceQueryService::Instance() {
    static WorkspaceQueryService service;
    return service;
}

int32_t WorkspaceQueryService::CurrentStructureId() const {
    return domain::GetWorkspaceStore().CurrentStructureId();
}

int32_t WorkspaceQueryService::SelectedMeshId() const {
    return domain::GetWorkspaceStore().SelectedMeshId();
}

int32_t WorkspaceQueryService::ActiveDensityStructureId() const {
    return domain::GetWorkspaceStore().ActiveDensityStructureId();
}

int32_t WorkspaceQueryService::ActiveDensityGridMeshId() const {
    return domain::GetWorkspaceStore().ActiveDensityGridMeshId();
}

WorkspaceQueryService& GetWorkspaceQueryService() {
    return WorkspaceQueryService::Instance();
}

} // namespace application
} // namespace workspace

