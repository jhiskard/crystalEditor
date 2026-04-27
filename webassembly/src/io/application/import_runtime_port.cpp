#include "import_runtime_port.h"

#include "../../density/presentation/charge_density_ui.h"
#include "../../shell/runtime/workbench_runtime.h"
#include "../../structure/application/structure_lifecycle_service.h"
#include "../../structure/application/structure_service.h"
#include "../../workspace/legacy/legacy_atoms_runtime.h"

#include <utility>

namespace io::application {

namespace {

class RuntimeImportPort final : public ImportRuntimePort {
public:
    bool HasStructures() const override {
        return GetWorkbenchRuntime().StructureFeature().GetStructureCount() > 0;
    }

    bool HasUnitCell() const override {
        return workspace::legacy::LegacyAtomsRuntime().hasUnitCell();
    }

    bool HasChargeDensity() const override {
        return workspace::legacy::LegacyAtomsRuntime().HasChargeDensity();
    }

    int32_t GetCurrentStructureId() const override {
        return GetWorkbenchRuntime().StructureFeature().GetCurrentStructureId();
    }

    int32_t GetChargeDensityStructureId() const override {
        return workspace::legacy::LegacyAtomsRuntime().GetChargeDensityStructureId();
    }

    std::string GetLoadedFileName() const override {
        return workspace::legacy::LegacyAtomsRuntime().GetLoadedFileName();
    }

    void SetCurrentStructureId(int32_t structureId) override {
        GetWorkbenchRuntime().StructureFeature().SetCurrentStructureId(structureId);
    }

    void SetChargeDensityStructureId(int32_t structureId) override {
        workspace::legacy::LegacyAtomsRuntime().SetChargeDensityStructureId(structureId);
    }

    void SetLoadedFileName(const std::string& fileName) override {
        workspace::legacy::LegacyAtomsRuntime().SetLoadedFileName(fileName);
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

    bool LoadXsfParsedData(
        const atoms::infrastructure::FileIOManager::ParseResult& parseResult,
        bool renderCell) override {
        return workspace::legacy::LegacyAtomsRuntime().LoadXSFParsedData(parseResult, renderCell);
    }

    bool LoadChgcarParsedData(
        const atoms::infrastructure::ChgcarParser::ParseResult& parseResult) override {
        return workspace::legacy::LegacyAtomsRuntime().LoadChgcarParsedData(parseResult);
    }

    void SetChargeDensityGridDataEntries(std::vector<ImportGridDataEntry>&& entries) override {
        if (auto* chargeDensityUi = workspace::legacy::LegacyAtomsRuntime().chargeDensityUI()) {
            std::vector<atoms::ui::ChargeDensityUI::GridDataEntry> converted;
            converted.reserve(entries.size());
            for (auto& entry : entries) {
                atoms::ui::ChargeDensityUI::GridDataEntry convertedEntry;
                convertedEntry.name = entry.name;
                convertedEntry.dims = entry.dims;
                convertedEntry.origin[0] = entry.origin[0];
                convertedEntry.origin[1] = entry.origin[1];
                convertedEntry.origin[2] = entry.origin[2];
                for (int i = 0; i < 3; ++i) {
                    for (int j = 0; j < 3; ++j) {
                        convertedEntry.vectors[i][j] = entry.vectors[i][j];
                    }
                }
                convertedEntry.values = std::move(entry.values);
                converted.push_back(std::move(convertedEntry));
            }
            chargeDensityUi->setGridDataEntries(std::move(converted));
        }
    }

    void SetAllChargeDensityAdvancedGridVisible(
        int32_t structureId,
        bool volumeMode,
        bool visible) override {
        workspace::legacy::LegacyAtomsRuntime().SetAllChargeDensityAdvancedGridVisible(
            structureId,
            volumeMode,
            visible);
    }

    AtomsTemplate* LegacyParent() override {
        return &workspace::legacy::LegacyAtomsRuntime();
    }
};

} // namespace

ImportRuntimePort& GetImportRuntimePort() {
    static RuntimeImportPort port;
    return port;
}

} // namespace io::application




