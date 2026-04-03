#include "mesh_repository.h"

#include "../../mesh_manager.h"

namespace {
MeshManager& Manager() {
    return MeshManager::Instance();
}

const MeshManager& ManagerConst() {
    return MeshManager::Instance();
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

size_t MeshRepository::MeshCount() const {
    return ManagerConst().GetMeshCount();
}

const Mesh* MeshRepository::FindMeshById(int32_t id) const {
    return ManagerConst().GetMeshById(id);
}

Mesh* MeshRepository::FindMeshByIdMutable(int32_t id) {
    return Manager().GetMeshByIdMutable(id);
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

bool MeshRepository::GetGlobalVolumeDataRange(double& minOut, double& maxOut) const {
    return ManagerConst().GetGlobalVolumeDataRange(minOut, maxOut);
}

MeshRepository& GetMeshRepository() {
    return MeshRepository::Instance();
}

} // namespace domain
} // namespace mesh

