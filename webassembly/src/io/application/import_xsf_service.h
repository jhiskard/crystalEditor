/**
 * @file import_xsf_service.h
 * @brief XSF parsing/apply bridge service for import workflow.
 */
#pragma once

#include "../infrastructure/file_io_manager.h"

#include <array>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace io::application {

/**
 * @brief Density grid payload transferred from XSF-grid import flow.
 */
struct XsfGridDataEntry {
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
 * @brief Application service that handles XSF parse/apply compatibility bridge.
 * @details Temporary compatibility path: parser/apply flow still delegates to
 *          legacy runtime-backed implementation during W5.6 transition.
 */
class ImportXsfService {
public:
    using ProgressCallback = std::function<void(float)>;

    bool ContainsDatagrid3d(const std::string& filePath) const;
    atoms::infrastructure::FileIOManager::ParseResult ParseXsfFile(
        const std::string& filePath,
        const ProgressCallback& progressCallback) const;
    atoms::infrastructure::FileIOManager::Grid3DParseResult ParseXsfGridFile(
        const std::string& filePath,
        const ProgressCallback& progressCallback) const;

    bool LoadXsfParsedData(
        const atoms::infrastructure::FileIOManager::ParseResult& parseResult,
        bool renderCell) const;

    void SetChargeDensityGridDataEntries(std::vector<XsfGridDataEntry>&& entries) const;
    void SetAllChargeDensityAdvancedGridVisible(
        int32_t structureId,
        bool volumeMode,
        bool visible) const;
};

} // namespace io::application
