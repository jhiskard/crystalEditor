/**
 * @file density_service.h
 * @brief Charge-density feature application service facade.
 */
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "density_service_types.h"

namespace atoms {
namespace ui {
class ChargeDensityUI;
} // namespace ui
} // namespace atoms

namespace density {
namespace application {

/**
 * @brief Density use-case facade extracted from AtomsTemplate.
 * @details Application layer delegates to legacy runtime in W5.3 thin-shim mode.
 */
class DensityService {
public:
    /**
     * @brief Creates density service facade.
     */
    DensityService();

    bool HasChargeDensity() const;
    bool IsChargeDensityVisible() const;
    void SetChargeDensityVisible(bool visible);
    bool IsSimpleViewActive() const;
    void SyncChargeDensityViewTypeState();

    int32_t GetChargeDensityStructureId() const;
    void SetChargeDensityStructureId(int32_t structureId);

    bool IsAdvancedGridVisible(int32_t meshId, bool volumeMode) const;
    void SetAdvancedGridVisible(int32_t meshId, bool volumeMode, bool visible);
    void SetAllAdvancedGridVisible(int32_t structureId, bool volumeMode, bool visible);
    void ApplyAdvancedGridVisibilityForStructure(int32_t structureId);

    std::vector<SimpleGridEntry> GetSimpleGridEntries() const;
    std::vector<SliceGridEntry> GetSliceGridEntries() const;
    bool IsSliceVisible() const;
    void SetAllSimpleGridVisible(bool visible);
    void SetAllSliceGridVisible(bool visible);
    bool SetSimpleGridVisible(const std::string& gridName, bool visible);
    bool SetSliceGridVisible(const std::string& gridName, bool visible);
    bool SelectSimpleGridByName(const std::string& gridName);
    bool SelectSliceGridByName(const std::string& gridName);

    void RenderChargeDensityViewerWindow(bool* openWindow = nullptr);
    void RenderSliceViewerWindow(bool* openWindow = nullptr);

    atoms::ui::ChargeDensityUI* ChargeDensityUi();
    const atoms::ui::ChargeDensityUI* ChargeDensityUi() const;

};

} // namespace application
} // namespace density



