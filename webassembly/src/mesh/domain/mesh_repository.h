/**
 * @file mesh_repository.h
 * @brief Mesh repository facade for Phase 10 decoupling.
 */
#pragma once

#include "mesh_repository_core.h"

#include <cstddef>
#include <cstdint>
#include <string>

// VTK
#include <vtkSmartPointer.h>

class vtkDataSet;

namespace mesh {
namespace domain {

/**
 * @brief Repository facade that centralizes mesh access/mutation.
 * @details This class wraps the legacy `MeshManager` singleton so callers can
 *          migrate to mesh-module contracts without immediate runtime behavior changes.
 */
class MeshRepository {
public:
    using VolumeDisplaySettings = MeshManager::VolumeDisplaySettings;

    /**
     * @brief Returns singleton repository facade instance.
     */
    static MeshRepository& Instance();

    Mesh* InsertMesh(const char* name,
        vtkSmartPointer<vtkDataSet> edgeDataSet = nullptr,
        vtkSmartPointer<vtkDataSet> faceDataSet = nullptr,
        vtkSmartPointer<vtkDataSet> volumeDataSet = nullptr,
        int32_t parentId = 0);

    const LcrsTreeUPtr& MeshTree() const;
    const LcrsTreeUPtr& GetMeshTree() const;
    size_t MeshCount() const;
    size_t GetMeshCount() const;
    const Mesh* FindMeshById(int32_t id) const;
    const Mesh* GetMeshById(int32_t id) const;
    Mesh* FindMeshByIdMutable(int32_t id);
    Mesh* GetMeshByIdMutable(int32_t id);

    void ShowMesh(int32_t id);
    void HideMesh(int32_t id);
    void DeleteMesh(int32_t id);
    void SetDisplayMode(int32_t id, MeshDisplayMode mode);
    void SetAllDisplayMode(MeshDisplayMode mode);

    int32_t RegisterXsfStructure(const std::string& fileName);
    void DeleteXsfStructure(int32_t id);
    void DeleteAllXsfStructures();
    bool HasXsfStructures() const;
    void SetParentIconState(TreeNode* node) const;

    bool GetGlobalVolumeDataRange(double& minOut, double& maxOut) const;
    bool HasSharedVolumeDisplaySettings() const;
    const VolumeDisplaySettings& GetSharedVolumeDisplaySettings() const;
    void SetSharedVolumeDisplaySettings(const VolumeDisplaySettings& settings);
    void EnsureSharedVolumeDisplaySettingsFromMesh(const Mesh& mesh);
    void ApplySharedVolumeDisplaySettingsToAllMeshes();

#ifdef DEBUG_BUILD
    void PrintMeshTree() const;
#endif

private:
    MeshRepository() = default;
};

/**
 * @brief Convenience accessor for mesh repository facade.
 */
MeshRepository& GetMeshRepository();

} // namespace domain
} // namespace mesh


