#include "import_runtime_port.h"

#include "../../shell/runtime/workbench_runtime.h"
#include "../../structure/application/structure_lifecycle_service.h"
#include "../../structure/application/structure_service.h"
#include "../../workspace/runtime/legacy_atoms_runtime.h"

namespace io::application {

namespace {

class RuntimeImportPort final : public ImportRuntimePort {
public:
    bool HasStructures() const override {
        return GetWorkbenchRuntime().StructureFeature().GetStructureCount() > 0;
    }

    bool HasUnitCell() const override {
        return workspace::legacy::WorkspaceRuntimeModelRef().hasUnitCell();
    }

    bool HasChargeDensity() const override {
        return workspace::legacy::WorkspaceRuntimeModelRef().HasChargeDensity();
    }

    int32_t GetCurrentStructureId() const override {
        return GetWorkbenchRuntime().StructureFeature().GetCurrentStructureId();
    }

    int32_t GetChargeDensityStructureId() const override {
        return workspace::legacy::WorkspaceRuntimeModelRef().GetChargeDensityStructureId();
    }

    std::string GetLoadedFileName() const override {
        return workspace::legacy::WorkspaceRuntimeModelRef().GetLoadedFileName();
    }

    void SetCurrentStructureId(int32_t structureId) override {
        GetWorkbenchRuntime().StructureFeature().SetCurrentStructureId(structureId);
    }

    void SetChargeDensityStructureId(int32_t structureId) override {
        workspace::legacy::WorkspaceRuntimeModelRef().SetChargeDensityStructureId(structureId);
    }

    void SetLoadedFileName(const std::string& fileName) override {
        workspace::legacy::WorkspaceRuntimeModelRef().SetLoadedFileName(fileName);
    }

    void RegisterStructure(int32_t structureId, const std::string& name) override {
        GetWorkbenchRuntime().StructureLifecycleFeature().RegisterStructure(structureId, name);
    }

    void RemoveStructure(int32_t structureId) override {
        GetWorkbenchRuntime().StructureLifecycleFeature().RemoveStructure(structureId);
    }

    void RemoveUnassignedData() override {
        GetWorkbenchRuntime().StructureLifecycleFeature().RemoveUnassignedData();
    }
};

} // namespace

ImportRuntimePort& GetImportRuntimePort() {
    static RuntimeImportPort port;
    return port;
}

} // namespace io::application




