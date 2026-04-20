/**
 * @file legacy_density_service_port.h
 * @brief Legacy density adapter implementing DensityServicePort.
 * @note Temporary compatibility path (remove in Phase18-W6, Track: P18-W3-DENSITY-PORT).
 */
#pragma once

#include "../../application/density_service_port.h"

namespace density {
namespace infrastructure {
namespace legacy {

/**
 * @brief Adapts density service port calls to legacy AtomsTemplate facade.
 */
class LegacyDensityServicePort final : public application::DensityServicePort {
public:
    bool HasChargeDensity() const override;
    bool IsChargeDensityVisible() const override;
    void SetChargeDensityVisible(bool visible) override;
    bool IsSimpleViewActive() const override;
    void SyncChargeDensityViewTypeState() override;

    int32_t GetChargeDensityStructureId() const override;
    void SetChargeDensityStructureId(int32_t structureId) override;

    bool IsAdvancedGridVisible(int32_t meshId, bool volumeMode) const override;
    void SetAdvancedGridVisible(int32_t meshId, bool volumeMode, bool visible) override;
    void SetAllAdvancedGridVisible(int32_t structureId, bool volumeMode, bool visible) override;
    void ApplyAdvancedGridVisibilityForStructure(int32_t structureId) override;

    atoms::ui::ChargeDensityUI* ChargeDensityUi() override;
    const atoms::ui::ChargeDensityUI* ChargeDensityUi() const override;
};

} // namespace legacy
} // namespace infrastructure
} // namespace density
