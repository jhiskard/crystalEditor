/**
 * @file import_runtime_port.h
 * @brief IO import compatibility port for legacy atoms runtime access.
 * @note Temporary compatibility path (remove in Phase18-W6 closeout, Track: P18-W4-IO-PORT).
 */
#pragma once

#include "../infrastructure/chgcar_parser.h"
#include "../infrastructure/file_io_manager.h"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

class AtomsTemplate;

namespace io::application {

/**
 * @brief Density grid payload passed from import application to runtime adapter.
 */
struct ImportGridDataEntry {
    std::string name;
    std::array<int, 3> dims { 0, 0, 0 };
    float origin[3] { 0.0f, 0.0f, 0.0f };
    float vectors[3][3] {
        { 0.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f }
    };
    std::vector<float> values;
};

/**
 * @brief Port contract for legacy runtime calls used by IO import application services.
 */
class ImportRuntimePort {
public:
    virtual ~ImportRuntimePort() = default;

    virtual bool HasStructures() const = 0;
    virtual bool HasUnitCell() const = 0;
    virtual bool HasChargeDensity() const = 0;

    virtual int32_t GetCurrentStructureId() const = 0;
    virtual int32_t GetChargeDensityStructureId() const = 0;
    virtual std::string GetLoadedFileName() const = 0;

    virtual void SetCurrentStructureId(int32_t structureId) = 0;
    virtual void SetChargeDensityStructureId(int32_t structureId) = 0;
    virtual void SetLoadedFileName(const std::string& fileName) = 0;
    virtual void RegisterStructure(int32_t structureId, const std::string& name) = 0;
    virtual void RemoveStructure(int32_t structureId) = 0;
    virtual void RemoveUnassignedData() = 0;

    virtual bool LoadXsfParsedData(
        const atoms::infrastructure::FileIOManager::ParseResult& parseResult,
        bool renderCell) = 0;
    virtual bool LoadChgcarParsedData(
        const atoms::infrastructure::ChgcarParser::ParseResult& parseResult) = 0;

    virtual void SetChargeDensityGridDataEntries(std::vector<ImportGridDataEntry>&& entries) = 0;
    virtual void SetAllChargeDensityAdvancedGridVisible(
        int32_t structureId, bool volumeMode, bool visible) = 0;

    virtual AtomsTemplate* LegacyParent() = 0;
};

/**
 * @brief Returns default import runtime port bound to legacy runtime adapter.
 */
ImportRuntimePort& GetImportRuntimePort();

} // namespace io::application
