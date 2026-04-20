#include "legacy_import_runtime_port.h"

#include "../../../density/presentation/charge_density_ui.h"
#include "../../../workspace/legacy/legacy_atoms_runtime.h"

#include <utility>

namespace io::infrastructure::legacy {

namespace {

// Keep io/application decoupled from presentation-specific grid entry types.
std::vector<atoms::ui::ChargeDensityUI::GridDataEntry> convertGridEntries(
    std::vector<application::ImportGridDataEntry>&& entries) {
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

    return converted;
}

} // namespace

bool LegacyImportRuntimePort::HasStructures() const {
    return workspace::legacy::LegacyAtomsRuntime().HasStructures();
}

bool LegacyImportRuntimePort::HasUnitCell() const {
    return workspace::legacy::LegacyAtomsRuntime().hasUnitCell();
}

bool LegacyImportRuntimePort::HasChargeDensity() const {
    return workspace::legacy::LegacyAtomsRuntime().HasChargeDensity();
}

int32_t LegacyImportRuntimePort::GetCurrentStructureId() const {
    return workspace::legacy::LegacyAtomsRuntime().GetCurrentStructureId();
}

int32_t LegacyImportRuntimePort::GetChargeDensityStructureId() const {
    return workspace::legacy::LegacyAtomsRuntime().GetChargeDensityStructureId();
}

std::string LegacyImportRuntimePort::GetLoadedFileName() const {
    return workspace::legacy::LegacyAtomsRuntime().GetLoadedFileName();
}

void LegacyImportRuntimePort::SetCurrentStructureId(int32_t structureId) {
    workspace::legacy::LegacyAtomsRuntime().SetCurrentStructureId(structureId);
}

void LegacyImportRuntimePort::SetChargeDensityStructureId(int32_t structureId) {
    workspace::legacy::LegacyAtomsRuntime().SetChargeDensityStructureId(structureId);
}

void LegacyImportRuntimePort::SetLoadedFileName(const std::string& fileName) {
    workspace::legacy::LegacyAtomsRuntime().SetLoadedFileName(fileName);
}

void LegacyImportRuntimePort::RegisterStructure(int32_t structureId, const std::string& name) {
    workspace::legacy::LegacyAtomsRuntime().RegisterStructure(structureId, name);
}

void LegacyImportRuntimePort::RemoveStructure(int32_t structureId) {
    workspace::legacy::LegacyAtomsRuntime().RemoveStructure(structureId);
}

void LegacyImportRuntimePort::RemoveUnassignedData() {
    workspace::legacy::LegacyAtomsRuntime().RemoveUnassignedData();
}

bool LegacyImportRuntimePort::LoadXsfParsedData(
    const atoms::infrastructure::FileIOManager::ParseResult& parseResult,
    bool renderCell) {
    return workspace::legacy::LegacyAtomsRuntime().LoadXSFParsedData(parseResult, renderCell);
}

bool LegacyImportRuntimePort::LoadChgcarParsedData(
    const atoms::infrastructure::ChgcarParser::ParseResult& parseResult) {
    return workspace::legacy::LegacyAtomsRuntime().LoadChgcarParsedData(parseResult);
}

void LegacyImportRuntimePort::SetChargeDensityGridDataEntries(
    std::vector<application::ImportGridDataEntry>&& entries) {
    if (auto* chargeDensityUi = workspace::legacy::LegacyAtomsRuntime().chargeDensityUI()) {
        chargeDensityUi->setGridDataEntries(convertGridEntries(std::move(entries)));
    }
}

void LegacyImportRuntimePort::SetAllChargeDensityAdvancedGridVisible(
    int32_t structureId, bool volumeMode, bool visible) {
    workspace::legacy::LegacyAtomsRuntime().SetAllChargeDensityAdvancedGridVisible(
        structureId, volumeMode, visible);
}

AtomsTemplate* LegacyImportRuntimePort::LegacyParent() {
    return &workspace::legacy::LegacyAtomsRuntime();
}

} // namespace io::infrastructure::legacy

namespace io::application {

ImportRuntimePort& GetImportRuntimePort() {
    static infrastructure::legacy::LegacyImportRuntimePort port;
    return port;
}

} // namespace io::application
