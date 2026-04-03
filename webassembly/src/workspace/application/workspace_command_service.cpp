#include "workspace_command_service.h"

#include "../domain/workspace_store.h"

namespace workspace {
namespace application {

WorkspaceCommandService& WorkspaceCommandService::Instance() {
    static WorkspaceCommandService service;
    return service;
}

void WorkspaceCommandService::Reset() {
    domain::GetWorkspaceStore().Reset();
}

void WorkspaceCommandService::SetCurrentStructureId(int32_t structureId) {
    domain::GetWorkspaceStore().SetCurrentStructureId(structureId);
}

void WorkspaceCommandService::SetSelectedMeshId(int32_t meshId) {
    domain::GetWorkspaceStore().SetSelectedMeshId(meshId);
}

void WorkspaceCommandService::SetActiveDensityStructureId(int32_t structureId) {
    domain::GetWorkspaceStore().SetActiveDensityStructureId(structureId);
}

void WorkspaceCommandService::SetActiveDensityGridMeshId(int32_t meshId) {
    domain::GetWorkspaceStore().SetActiveDensityGridMeshId(meshId);
}

WorkspaceCommandService& GetWorkspaceCommandService() {
    return WorkspaceCommandService::Instance();
}

} // namespace application
} // namespace workspace

