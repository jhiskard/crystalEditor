/**
 * @file legacy_import_runtime_port.h
 * @brief Legacy adapter implementing io::application::ImportRuntimePort.
 * @note Temporary compatibility path (remove in Phase18-W6 closeout, Track: P18-W4-IO-PORT).
 */
#pragma once

#include "../../application/import_runtime_port.h"

namespace io::infrastructure::legacy {

/**
 * @brief Bridges import runtime port calls to workspace legacy runtime facade.
 */
class LegacyImportRuntimePort final : public application::ImportRuntimePort {
public:
    bool HasStructures() const override;
    bool HasUnitCell() const override;
    bool HasChargeDensity() const override;

    int32_t GetCurrentStructureId() const override;
    int32_t GetChargeDensityStructureId() const override;
    std::string GetLoadedFileName() const override;

    void SetCurrentStructureId(int32_t structureId) override;
    void SetChargeDensityStructureId(int32_t structureId) override;
    void SetLoadedFileName(const std::string& fileName) override;
    void RegisterStructure(int32_t structureId, const std::string& name) override;
    void RemoveStructure(int32_t structureId) override;
    void RemoveUnassignedData() override;

    bool LoadXsfParsedData(
        const atoms::infrastructure::FileIOManager::ParseResult& parseResult,
        bool renderCell) override;
    bool LoadChgcarParsedData(
        const atoms::infrastructure::ChgcarParser::ParseResult& parseResult) override;

    void SetChargeDensityGridDataEntries(std::vector<application::ImportGridDataEntry>&& entries) override;
    void SetAllChargeDensityAdvancedGridVisible(
        int32_t structureId, bool volumeMode, bool visible) override;

    AtomsTemplate* LegacyParent() override;
};

} // namespace io::infrastructure::legacy
