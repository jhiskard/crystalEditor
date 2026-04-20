#include "import_workflow_service.h"

#include "import_runtime_port.h"
#include "../../mesh/application/mesh_command_service.h"
#include "../../mesh/application/mesh_query_service.h"
#include "../../workspace/application/workspace_command_service.h"

namespace io::application {

bool ImportWorkflowService::HasSceneDataForStructureImport() const {
    return m_ImportOrchestrator.HasSceneDataForStructureImport();
}

void ImportWorkflowService::BeginReplaceSceneImportTransaction() {
    m_ReplaceSceneImportSnapshot = m_ImportOrchestrator.BeginReplaceSceneImportTransaction();
    m_ReplaceSceneImportTransactionActive = true;
}

void ImportWorkflowService::FinalizeImportOnSuccess(int32_t importedStructureId) {
    if (!m_ReplaceSceneImportTransactionActive) {
        return;
    }

    m_ImportOrchestrator.FinalizeReplaceSceneImportSuccess(
        m_ReplaceSceneImportSnapshot,
        importedStructureId);

    workspace::application::WorkspaceCommandService& workspaceCommand =
        workspace::application::GetWorkspaceCommandService();
    workspaceCommand.SetCurrentStructureId(importedStructureId);
    workspaceCommand.SetSelectedMeshId(importedStructureId);

    ClearReplaceSceneImportTransaction();
}

void ImportWorkflowService::RollbackImportOnFailure(int32_t importedStructureId) {
    if (!m_ReplaceSceneImportTransactionActive) {
        return;
    }

    m_ImportOrchestrator.RollbackFailedStructureImport(
        m_ReplaceSceneImportSnapshot,
        importedStructureId);

    workspace::application::WorkspaceCommandService& workspaceCommand =
        workspace::application::GetWorkspaceCommandService();
    workspaceCommand.SetCurrentStructureId(m_ReplaceSceneImportSnapshot.currentStructureId);
    workspaceCommand.SetActiveDensityStructureId(m_ReplaceSceneImportSnapshot.chargeDensityStructureId);
    workspaceCommand.SetSelectedMeshId(m_ReplaceSceneImportSnapshot.currentStructureId);

    ClearReplaceSceneImportTransaction();
}

void ImportWorkflowService::CleanupImportedStructure(int32_t importedStructureId) const {
    if (importedStructureId < 0) {
        return;
    }

    mesh::application::MeshQueryService& meshQuery = mesh::application::GetMeshQueryService();
    mesh::application::MeshCommandService& meshCommand = mesh::application::GetMeshCommandService();
    ImportRuntimePort& importRuntime = GetImportRuntimePort();

    const Mesh* importedMesh = meshQuery.FindMeshById(importedStructureId);
    if (importedMesh == nullptr) {
        return;
    }

    if (importedMesh->IsXsfStructure()) {
        importRuntime.RemoveStructure(importedStructureId);
        meshCommand.DeleteXsfStructure(importedStructureId);
    } else {
        meshCommand.DeleteMesh(importedStructureId);
    }
}

void ImportWorkflowService::ClearReplaceSceneImportTransaction() {
    m_ReplaceSceneImportTransactionActive = false;
    m_ReplaceSceneImportSnapshot.Clear();
}

} // namespace io::application

