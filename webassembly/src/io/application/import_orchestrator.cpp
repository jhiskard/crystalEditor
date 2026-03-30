#include "import_orchestrator.h"

#include "../../atoms/atoms_template.h"
#include "../../atoms/domain/atom_manager.h"
#include "../../mesh_manager.h"

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
    if (!atoms::domain::createdAtoms.empty() || !atoms::domain::surroundingAtoms.empty()) {
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

    const LcrsTreeUPtr& meshTree = MeshManager::Instance().GetMeshTree();
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
    MeshManager& meshManager = MeshManager::Instance();
    AtomsTemplate& atomsTemplate = AtomsTemplate::Instance();

    for (int32_t rootMeshId : snapshot.rootMeshIds) {
        if (rootMeshId == importedStructureId) {
            continue;
        }

        const Mesh* mesh = meshManager.GetMeshById(rootMeshId);
        if (!mesh) {
            continue;
        }

        if (mesh->IsXsfStructure()) {
            atomsTemplate.RemoveStructure(rootMeshId);
            meshManager.DeleteXsfStructure(rootMeshId);
        } else {
            meshManager.DeleteMesh(rootMeshId);
        }
    }

    atomsTemplate.RemoveUnassignedData();
    if (importedStructureId >= 0 && meshManager.GetMeshById(importedStructureId) != nullptr) {
        atomsTemplate.SetCurrentStructureId(importedStructureId);
    }
}

void ImportOrchestrator::RollbackFailedStructureImport(
    const ReplaceSceneImportSnapshot& snapshot,
    int32_t importedStructureId) const {
    MeshManager& meshManager = MeshManager::Instance();
    AtomsTemplate& atomsTemplate = AtomsTemplate::Instance();

    if (importedStructureId >= 0) {
        const Mesh* importedMesh = meshManager.GetMeshById(importedStructureId);
        if (importedMesh != nullptr) {
            if (importedMesh->IsXsfStructure()) {
                atomsTemplate.RemoveStructure(importedStructureId);
                meshManager.DeleteXsfStructure(importedStructureId);
            } else {
                meshManager.DeleteMesh(importedStructureId);
            }
        }
    }

    atomsTemplate.SetLoadedFileName(snapshot.loadedFileName);

    const auto isValidStructureId = [&meshManager](int32_t structureId) {
        if (structureId < 0) {
            return false;
        }
        const Mesh* mesh = meshManager.GetMeshById(structureId);
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
