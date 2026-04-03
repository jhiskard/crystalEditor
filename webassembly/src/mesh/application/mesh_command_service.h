/**
 * @file mesh_command_service.h
 * @brief Mesh mutation command service.
 */
#pragma once

#include "../../mesh.h"

#include <cstdint>
#include <string>

// VTK
#include <vtkSmartPointer.h>

class vtkDataSet;

namespace mesh {
namespace application {

/**
 * @brief Command facade for mesh state mutations.
 * @details This service isolates write operations from presentation/runtime
 *          call sites during the MeshManager compatibility transition.
 */
class MeshCommandService {
public:
    /**
     * @brief Returns singleton command service instance.
     */
    static MeshCommandService& Instance();

    Mesh* InsertMesh(const char* name,
        vtkSmartPointer<vtkDataSet> edgeDataSet = nullptr,
        vtkSmartPointer<vtkDataSet> faceDataSet = nullptr,
        vtkSmartPointer<vtkDataSet> volumeDataSet = nullptr,
        int32_t parentId = 0);

    void ShowMesh(int32_t id);
    void HideMesh(int32_t id);
    void DeleteMesh(int32_t id);
    void SetDisplayMode(int32_t id, MeshDisplayMode mode);
    void SetAllDisplayMode(MeshDisplayMode mode);

    int32_t RegisterXsfStructure(const std::string& fileName);
    void DeleteXsfStructure(int32_t id);
    void DeleteAllXsfStructures();

private:
    MeshCommandService() = default;
};

/**
 * @brief Convenience accessor for mesh command service.
 */
MeshCommandService& GetMeshCommandService();

} // namespace application
} // namespace mesh
