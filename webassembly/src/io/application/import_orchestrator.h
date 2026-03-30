#pragma once

// Standard library
#include <cstdint>
#include <string>
#include <vector>

namespace io::application {

struct ReplaceSceneImportSnapshot {
    std::vector<int32_t> rootMeshIds;
    int32_t currentStructureId = -1;
    int32_t chargeDensityStructureId = -1;
    std::string loadedFileName;

    void Clear();
};

class ImportOrchestrator {
public:
    bool HasSceneDataForStructureImport() const;
    ReplaceSceneImportSnapshot BeginReplaceSceneImportTransaction() const;
    void FinalizeReplaceSceneImportSuccess(
        const ReplaceSceneImportSnapshot& snapshot,
        int32_t importedStructureId) const;
    void RollbackFailedStructureImport(
        const ReplaceSceneImportSnapshot& snapshot,
        int32_t importedStructureId) const;

private:
    std::vector<int32_t> CollectRootMeshIds() const;
};

} // namespace io::application
