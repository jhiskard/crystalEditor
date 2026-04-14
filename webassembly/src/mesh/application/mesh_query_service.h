/**
 * @file mesh_query_service.h
 * @brief Read-only mesh query service.
 */
#pragma once

#include "../domain/lcrs_tree.h"
#include "../domain/mesh_entity.h"

#include <cstddef>
#include <cstdint>

namespace mesh {
namespace application {

/**
 * @brief Query facade for mesh read access.
 * @details Presenters and workflows should use this API instead of reading
 *          the legacy mesh core singleton directly.
 */
class MeshQueryService {
public:
    /**
     * @brief Returns singleton query service instance.
     */
    static MeshQueryService& Instance();

    const LcrsTreeUPtr& MeshTree() const;
    size_t MeshCount() const;
    const Mesh* FindMeshById(int32_t id) const;
    Mesh* FindMeshByIdMutable(int32_t id) const;
    bool HasXsfStructures() const;
    bool GetGlobalVolumeDataRange(double& minOut, double& maxOut) const;

private:
    MeshQueryService() = default;
};

/**
 * @brief Convenience accessor for mesh query service.
 */
MeshQueryService& GetMeshQueryService();

} // namespace application
} // namespace mesh



