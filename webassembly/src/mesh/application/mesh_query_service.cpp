#include "mesh_query_service.h"

#include "../domain/mesh_repository.h"

namespace mesh {
namespace application {

MeshQueryService& MeshQueryService::Instance() {
    static MeshQueryService service;
    return service;
}

const LcrsTreeUPtr& MeshQueryService::MeshTree() const {
    return domain::GetMeshRepository().MeshTree();
}

size_t MeshQueryService::MeshCount() const {
    return domain::GetMeshRepository().MeshCount();
}

const Mesh* MeshQueryService::FindMeshById(int32_t id) const {
    return domain::GetMeshRepository().FindMeshById(id);
}

Mesh* MeshQueryService::FindMeshByIdMutable(int32_t id) const {
    return domain::GetMeshRepository().FindMeshByIdMutable(id);
}

bool MeshQueryService::HasXsfStructures() const {
    return domain::GetMeshRepository().HasXsfStructures();
}

bool MeshQueryService::GetGlobalVolumeDataRange(double& minOut, double& maxOut) const {
    return domain::GetMeshRepository().GetGlobalVolumeDataRange(minOut, maxOut);
}

MeshQueryService& GetMeshQueryService() {
    return MeshQueryService::Instance();
}

} // namespace application
} // namespace mesh

