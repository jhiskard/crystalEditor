#include "import_orchestrator.h"

#include "../../workspace/legacy/atoms_template_facade.h"
#include "../../mesh/application/mesh_command_service.h"
#include "../../mesh/application/mesh_query_service.h"
#include "../../shell/runtime/workbench_runtime.h"
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

    const AtomsTemplate& atomsTemplate = AtomsTemplate::Instance();
    if (atomsTemplate.HasStructures()) {
        return true;
    }
    structure::domain::StructureRepository& structureRepository =
        structure::domain::GetStructureRepository();
    if (!structureRepository.CreatedAtoms().empty() || !structureRepository.SurroundingAtoms().empty()) {
        return true;
    }
    if (atomsTemplate.hasUnitCell()) {
        return true;
    }
    if (atomsTemplate.HasChargeDensity()) {
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

    const AtomsTemplate& atomsTemplate = AtomsTemplate::Instance();
    snapshot.currentStructureId = atomsTemplate.GetCurrentStructureId();
    snapshot.chargeDensityStructureId = atomsTemplate.GetChargeDensityStructureId();
    snapshot.loadedFileName = atomsTemplate.GetLoadedFileName();
    return snapshot;
}

void ImportOrchestrator::FinalizeReplaceSceneImportSuccess(
    const ReplaceSceneImportSnapshot& snapshot,
    int32_t importedStructureId) const {
    mesh::application::MeshQueryService& meshQuery = mesh::application::GetMeshQueryService();
    mesh::application::MeshCommandService& meshCommand = mesh::application::GetMeshCommandService();
    AtomsTemplate& atomsTemplate = AtomsTemplate::Instance();

    for (int32_t rootMeshId : snapshot.rootMeshIds) {
        if (rootMeshId == importedStructureId) {
            continue;
        }

        const Mesh* mesh = meshQuery.FindMeshById(rootMeshId);
        if (!mesh) {
            continue;
        }

        if (mesh->IsXsfStructure()) {
            atomsTemplate.RemoveStructure(rootMeshId);
            meshCommand.DeleteXsfStructure(rootMeshId);
        } else {
            meshCommand.DeleteMesh(rootMeshId);
        }
    }

    atomsTemplate.RemoveUnassignedData();
    if (importedStructureId >= 0 && meshQuery.FindMeshById(importedStructureId) != nullptr) {
        atomsTemplate.SetCurrentStructureId(importedStructureId);
    }
}

void ImportOrchestrator::RollbackFailedStructureImport(
    const ReplaceSceneImportSnapshot& snapshot,
    int32_t importedStructureId) const {
    mesh::application::MeshQueryService& meshQuery = mesh::application::GetMeshQueryService();
    mesh::application::MeshCommandService& meshCommand = mesh::application::GetMeshCommandService();
    AtomsTemplate& atomsTemplate = AtomsTemplate::Instance();

    if (importedStructureId >= 0) {
        const Mesh* importedMesh = meshQuery.FindMeshById(importedStructureId);
        if (importedMesh != nullptr) {
            if (importedMesh->IsXsfStructure()) {
                atomsTemplate.RemoveStructure(importedStructureId);
                meshCommand.DeleteXsfStructure(importedStructureId);
            } else {
                meshCommand.DeleteMesh(importedStructureId);
            }
        }
    }

    atomsTemplate.SetLoadedFileName(snapshot.loadedFileName);

    const auto isValidStructureId = [&meshQuery](int32_t structureId) {
        if (structureId < 0) {
            return false;
        }
        const Mesh* mesh = meshQuery.FindMeshById(structureId);
        return mesh != nullptr && mesh->IsXsfStructure();
    };

    if (isValidStructureId(snapshot.currentStructureId)) {
        atomsTemplate.SetCurrentStructureId(snapshot.currentStructureId);
    } else {
        atomsTemplate.SetCurrentStructureId(-1);
    }

    if (isValidStructureId(snapshot.chargeDensityStructureId)) {
        atomsTemplate.SetChargeDensityStructureId(snapshot.chargeDensityStructureId);
    } else {
        atomsTemplate.SetChargeDensityStructureId(-1);
    }
}

} // namespace io::application

