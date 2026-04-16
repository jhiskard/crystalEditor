/**
 * @file density_service_port.h
 * @brief Port interface for density service compatibility backends.
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
 * @brief Port abstraction used by DensityService.
 */
class DensityServicePort {
public:
    virtual ~DensityServicePort() = default;

    virtual bool HasChargeDensity() const = 0;
    virtual bool IsChargeDensityVisible() const = 0;
    virtual void SetChargeDensityVisible(bool visible) = 0;
    virtual bool IsSimpleViewActive() const = 0;
    virtual void SyncChargeDensityViewTypeState() = 0;

    virtual int32_t GetChargeDensityStructureId() const = 0;
    virtual void SetChargeDensityStructureId(int32_t structureId) = 0;

    virtual bool IsAdvancedGridVisible(int32_t meshId, bool volumeMode) const = 0;
    virtual void SetAdvancedGridVisible(int32_t meshId, bool volumeMode, bool visible) = 0;
    virtual void SetAllAdvancedGridVisible(int32_t structureId, bool volumeMode, bool visible) = 0;
    virtual void ApplyAdvancedGridVisibilityForStructure(int32_t structureId) = 0;

    virtual atoms::ui::ChargeDensityUI* ChargeDensityUi() = 0;
    virtual const atoms::ui::ChargeDensityUI* ChargeDensityUi() const = 0;
};

} // namespace application
} // namespace density

