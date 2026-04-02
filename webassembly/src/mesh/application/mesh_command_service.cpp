#include "mesh_command_service.h"

#include "../domain/mesh_repository.h"

namespace mesh {
namespace application {

MeshCommandService& MeshCommandService::Instance() {
    static MeshCommandService service;
    return service;
}

Mesh* MeshCommandService::InsertMesh(const char* name,
    vtkSmartPointer<vtkDataSet> edgeDataSet,
    vtkSmartPointer<vtkDataSet> faceDataSet,
    vtkSmartPointer<vtkDataSet> volumeDataSet,
    int32_t parentId) {
    return domain::GetMeshRepository().InsertMesh(name, edgeDataSet, faceDataSet, volumeDataSet, parentId);
}

void MeshCommandService::ShowMesh(int32_t id) {
    domain::GetMeshRepository().ShowMesh(id);
}

void MeshCommandService::HideMesh(int32_t id) {
    domain::GetMeshRepository().HideMesh(id);
}

void MeshCommandService::DeleteMesh(int32_t id) {
    domain::GetMeshRepository().DeleteMesh(id);
}

void MeshCommandService::SetDisplayMode(int32_t id, MeshDisplayMode mode) {
    domain::GetMeshRepository().SetDisplayMode(id, mode);
}

void MeshCommandService::SetAllDisplayMode(MeshDisplayMode mode) {
    domain::GetMeshRepository().SetAllDisplayMode(mode);
}

int32_t MeshCommandService::RegisterXsfStructure(const std::string& fileName) {
    return domain::GetMeshRepository().RegisterXsfStructure(fileName);
}

void MeshCommandService::DeleteXsfStructure(int32_t id) {
    domain::GetMeshRepository().DeleteXsfStructure(id);
}

void MeshCommandService::DeleteAllXsfStructures() {
    domain::GetMeshRepository().DeleteAllXsfStructures();
}

MeshCommandService& GetMeshCommandService() {
    return MeshCommandService::Instance();
}

} // namespace application
} // namespace mesh

