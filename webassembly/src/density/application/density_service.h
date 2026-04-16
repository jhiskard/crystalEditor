/**
 * @file density_service.h
 * @brief Charge-density feature application service facade.
 */
#pragma once

#include <cstdint>

namespace atoms {
namespace ui {
class ChargeDensityUI;
} // namespace ui
} // namespace atoms

namespace density {
namespace application {

/**
 * @brief Density use-case facade extracted from AtomsTemplate.
 * @details R6 단계에서 compatibility port 의존을 제거하고 서비스가 직접
 *          legacy atoms runtime 경로를 호출하도록 전환했다.
 */
class DensityService {
public:
    /**
     * @brief Creates density service.
     */
    DensityService() = default;

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

    atoms::ui::ChargeDensityUI* ChargeDensityUi();
    const atoms::ui::ChargeDensityUI* ChargeDensityUi() const;

};

} // namespace application
} // namespace density

