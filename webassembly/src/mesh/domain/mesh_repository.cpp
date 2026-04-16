#include "mesh_repository.h"

#include "mesh_repository_core.h"

#include <iostream>

namespace {
MeshManager& Manager() {
    return MeshManager::Instance();
}

const MeshManager& ManagerConst() {
    return Manager();
}
} // namespace

namespace mesh {
namespace domain {

MeshRepository& MeshRepository::Instance() {
    static MeshRepository repository;
    return repository;
}

Mesh* MeshRepository::InsertMesh(const char* name,
    vtkSmartPointer<vtkDataSet> edgeDataSet,
    vtkSmartPointer<vtkDataSet> faceDataSet,
    vtkSmartPointer<vtkDataSet> volumeDataSet,
    int32_t parentId) {
    return Manager().InsertMesh(name, edgeDataSet, faceDataSet, volumeDataSet, parentId);
}

const LcrsTreeUPtr& MeshRepository::MeshTree() const {
    return ManagerConst().GetMeshTree();
}

const LcrsTreeUPtr& MeshRepository::GetMeshTree() const {
    return MeshTree();
}

size_t MeshRepository::MeshCount() const {
    return ManagerConst().GetMeshCount();
}

size_t MeshRepository::GetMeshCount() const {
    return MeshCount();
}

const Mesh* MeshRepository::FindMeshById(int32_t id) const {
    return ManagerConst().GetMeshById(id);
}

const Mesh* MeshRepository::GetMeshById(int32_t id) const {
    return FindMeshById(id);
}

Mesh* MeshRepository::FindMeshByIdMutable(int32_t id) {
    return Manager().GetMeshByIdMutable(id);
}

Mesh* MeshRepository::GetMeshByIdMutable(int32_t id) {
    return FindMeshByIdMutable(id);
}

void MeshRepository::ShowMesh(int32_t id) {
    Manager().ShowMesh(id);
}

void MeshRepository::HideMesh(int32_t id) {
    Manager().HideMesh(id);
}

void MeshRepository::DeleteMesh(int32_t id) {
    Manager().DeleteMesh(id);
}

void MeshRepository::SetDisplayMode(int32_t id, MeshDisplayMode mode) {
    Manager().SetDisplayMode(id, mode);
}

void MeshRepository::SetAllDisplayMode(MeshDisplayMode mode) {
    Manager().SetAllDisplayMode(mode);
}

int32_t MeshRepository::RegisterXsfStructure(const std::string& fileName) {
    return Manager().RegisterXsfStructure(fileName);
}

void MeshRepository::DeleteXsfStructure(int32_t id) {
    Manager().DeleteXsfStructure(id);
}

void MeshRepository::DeleteAllXsfStructures() {
    Manager().DeleteAllXsfStructures();
}

bool MeshRepository::HasXsfStructures() const {
    return ManagerConst().HasXsfStructures();
}

void MeshRepository::SetParentIconState(TreeNode* node) const {
    MeshManager::SetParentIconState(node);
}

bool MeshRepository::GetGlobalVolumeDataRange(double& minOut, double& maxOut) const {
    return ManagerConst().GetGlobalVolumeDataRange(minOut, maxOut);
}

bool MeshRepository::HasSharedVolumeDisplaySettings() const {
    return ManagerConst().HasSharedVolumeDisplaySettings();
}

const MeshRepository::VolumeDisplaySettings& MeshRepository::GetSharedVolumeDisplaySettings() const {
    return ManagerConst().GetSharedVolumeDisplaySettings();
}

void MeshRepository::SetSharedVolumeDisplaySettings(const VolumeDisplaySettings& settings) {
    Manager().SetSharedVolumeDisplaySettings(settings);
}

void MeshRepository::EnsureSharedVolumeDisplaySettingsFromMesh(const Mesh& mesh) {
    Manager().EnsureSharedVolumeDisplaySettingsFromMesh(mesh);
}

void MeshRepository::ApplySharedVolumeDisplaySettingsToAllMeshes() {
    Manager().ApplySharedVolumeDisplaySettingsToAllMeshes();
}

#ifdef DEBUG_BUILD
void MeshRepository::PrintMeshTree() const {
    ManagerConst().GetMeshTree()->TraverseTree([](const TreeNode* node, void*) {
        for (int i = 0; i < node->GetDepth(); ++i) {
            std::cout << "--";
        }
        std::cout << "[Node]: " << node->GetLabel() << ", [ID]: " << node->GetId();
        std::cout << ", [Parent]: " << (node->GetParent() ? node->GetParent()->GetLabel() : "NULL");
        std::cout << ", [Left Child]: " << (node->GetLeftChild() ? node->GetLeftChild()->GetLabel() : "NULL");
        std::cout << ", [Right Sibling]: " << (node->GetRightSibling() ? node->GetRightSibling()->GetLabel() : "NULL");
        std::cout << ", [Left Sibling]: " << (node->GetLeftSibling() ? node->GetLeftSibling()->GetLabel() : "NULL");
        std::cout << std::endl;
    });
}
#endif

MeshRepository& GetMeshRepository() {
    return MeshRepository::Instance();
}

} // namespace domain
} // namespace mesh




