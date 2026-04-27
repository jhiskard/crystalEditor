#include "import_xsf_service.h"

#include "../../density/presentation/charge_density_ui.h"
#include "../../workspace/runtime/legacy_atoms_runtime.h"

#include <fstream>
#include <utility>

namespace io::application {

bool ImportXsfService::ContainsDatagrid3d(const std::string& filePath) const {
    std::ifstream fin(filePath);
    if (!fin) {
        return false;
    }

    std::string line;
    while (std::getline(fin, line)) {
        if (line.find("DATAGRID_3D") != std::string::npos) {
            return true;
        }
    }
    return false;
}

atoms::infrastructure::FileIOManager::ParseResult ImportXsfService::ParseXsfFile(
    const std::string& filePath,
    const ProgressCallback& progressCallback) const {
    atoms::infrastructure::FileIOManager loader(&workspace::legacy::WorkspaceRuntimeModelRef());
    if (progressCallback) {
        loader.SetProgressCallback(progressCallback);
    }
    return loader.loadXSFFile(filePath);
}

atoms::infrastructure::FileIOManager::Grid3DParseResult ImportXsfService::ParseXsfGridFile(
    const std::string& filePath,
    const ProgressCallback& progressCallback) const {
    atoms::infrastructure::FileIOManager loader(&workspace::legacy::WorkspaceRuntimeModelRef());
    if (progressCallback) {
        loader.SetProgressCallback(progressCallback);
    }
    return loader.load3DGridXSFFile(filePath);
}

bool ImportXsfService::LoadXsfParsedData(
    const atoms::infrastructure::FileIOManager::ParseResult& parseResult,
    bool renderCell) const {
    return workspace::legacy::WorkspaceRuntimeModelRef().LoadXSFParsedData(parseResult, renderCell);
}

void ImportXsfService::SetChargeDensityGridDataEntries(std::vector<XsfGridDataEntry>&& entries) const {
    auto* chargeDensityUi = workspace::legacy::WorkspaceRuntimeModelRef().chargeDensityUI();
    if (chargeDensityUi == nullptr) {
        return;
    }

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

void ImportXsfService::SetAllChargeDensityAdvancedGridVisible(
    int32_t structureId,
    bool volumeMode,
    bool visible) const {
    workspace::legacy::WorkspaceRuntimeModelRef().SetAllChargeDensityAdvancedGridVisible(
        structureId,
        volumeMode,
        visible);
}

} // namespace io::application
