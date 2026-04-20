#include "import_orchestrator.h"

#include "import_runtime_port.h"
#include "../../mesh/application/mesh_command_service.h"
#include "../../mesh/application/mesh_query_service.h"
#include "../../structure/domain/structure_repository.h"

namespace io::application {

void ReplaceSceneImportSnapshot::Clear() {
    rootMeshIds.clear();
    currentStructureId = -1;
    chargeDensityStructureId = -1;
    loadedFileName.clear();
}

bool ImportOrchestrator::HasSceneDataForStructureImport() const {
    if (!CollectRootMeshIds().empty()) {
        return true;
    }

    const ImportRuntimePort& importRuntime = GetImportRuntimePort();
    if (importRuntime.HasStructures()) {
        return true;
    }
    structure::domain::StructureRepository& structureRepository =
        structure::domain::GetStructureRepository();
    if (!structureRepository.CreatedAtoms().empty() || !structureRepository.SurroundingAtoms().empty()) {
        return true;
    }
    if (importRuntime.HasUnitCell()) {
        return true;
    }
    if (importRuntime.HasChargeDensity()) {
        return true;
    }

    return false;
}

std::vector<int32_t> ImportOrchestrator::CollectRootMeshIds() const {
    std::vector<int32_t> rootMeshIds;
    const mesh::application::MeshQueryService& meshQuery = mesh::application::GetMeshQueryService();

    const LcrsTreeUPtr& meshTree = meshQuery.MeshTree();
    if (!meshTree) {
        return rootMeshIds;
    }
    const TreeNode* root = meshTree->GetRoot();
    if (!root) {
        return rootMeshIds;
    }

    const TreeNode* child = root->GetLeftChild();
    while (child != nullptr) {
        rootMeshIds.push_back(child->GetId());
        child = child->GetRightSibling();
    }
    return rootMeshIds;
}

ReplaceSceneImportSnapshot ImportOrchestrator::BeginReplaceSceneImportTransaction() const {
    ReplaceSceneImportSnapshot snapshot;
    snapshot.rootMeshIds = CollectRootMeshIds();

    const ImportRuntimePort& importRuntime = GetImportRuntimePort();
    snapshot.currentStructureId = importRuntime.GetCurrentStructureId();
    snapshot.chargeDensityStructureId = importRuntime.GetChargeDensityStructureId();
    snapshot.loadedFileName = importRuntime.GetLoadedFileName();
    return snapshot;
}

void ImportOrchestrator::FinalizeReplaceSceneImportSuccess(
    const ReplaceSceneImportSnapshot& snapshot,
    int32_t importedStructureId) const {
    mesh::application::MeshQueryService& meshQuery = mesh::application::GetMeshQueryService();
    mesh::application::MeshCommandService& meshCommand = mesh::application::GetMeshCommandService();
    ImportRuntimePort& importRuntime = GetImportRuntimePort();

    for (int32_t rootMeshId : snapshot.rootMeshIds) {
        if (rootMeshId == importedStructureId) {
            continue;
        }

        const Mesh* mesh = meshQuery.FindMeshById(rootMeshId);
        if (!mesh) {
            continue;
        }

        if (mesh->IsXsfStructure()) {
            importRuntime.RemoveStructure(rootMeshId);
            meshCommand.DeleteXsfStructure(rootMeshId);
        } else {
            meshCommand.DeleteMesh(rootMeshId);
        }
    }

    importRuntime.RemoveUnassignedData();
    if (importedStructureId >= 0 && meshQuery.FindMeshById(importedStructureId) != nullptr) {
        importRuntime.SetCurrentStructureId(importedStructureId);
    }
}

void ImportOrchestrator::RollbackFailedStructureImport(
    const ReplaceSceneImportSnapshot& snapshot,
    int32_t importedStructureId) const {
    mesh::application::MeshQueryService& meshQuery = mesh::application::GetMeshQueryService();
    mesh::application::MeshCommandService& meshCommand = mesh::application::GetMeshCommandService();
    ImportRuntimePort& importRuntime = GetImportRuntimePort();

    if (importedStructureId >= 0) {
        const Mesh* importedMesh = meshQuery.FindMeshById(importedStructureId);
        if (importedMesh != nullptr) {
            if (importedMesh->IsXsfStructure()) {
                importRuntime.RemoveStructure(importedStructureId);
                meshCommand.DeleteXsfStructure(importedStructureId);
            } else {
                meshCommand.DeleteMesh(importedStructureId);
            }
        }
    }

    importRuntime.SetLoadedFileName(snapshot.loadedFileName);

    const auto isValidStructureId = [&meshQuery](int32_t structureId) {
        if (structureId < 0) {
            return false;
        }
        const Mesh* mesh = meshQuery.FindMeshById(structureId);
        return mesh != nullptr && mesh->IsXsfStructure();
    };

    if (isValidStructureId(snapshot.currentStructureId)) {
        importRuntime.SetCurrentStructureId(snapshot.currentStructureId);
    } else {
        importRuntime.SetCurrentStructureId(-1);
    }

    if (isValidStructureId(snapshot.chargeDensityStructureId)) {
        importRuntime.SetChargeDensityStructureId(snapshot.chargeDensityStructureId);
    } else {
        importRuntime.SetChargeDensityStructureId(-1);
    }
}

} // namespace io::application
