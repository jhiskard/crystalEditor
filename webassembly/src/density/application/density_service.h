/**
 * @file density_service.h
 * @brief Charge-density feature application service facade.
 */
#pragma once

#include <cstdint>
#include <string>

class AtomsTemplate;

namespace atoms {
namespace ui {
class ChargeDensityUI;
} // namespace ui
} // namespace atoms

namespace density {
namespace application {

/**
 * @brief Density use-case facade extracted from AtomsTemplate.
 * @details During Phase 8 this class delegates to existing AtomsTemplate APIs
 *          to preserve behavior while call-sites migrate to module boundaries.
 */
class DensityService {
public:
    /**
     * @brief Creates service bound to the atoms compatibility facade.
     */
    explicit DensityService(AtomsTemplate* atomsTemplate);

    bool HasChargeDensity() const;
    bool IsChargeDensityVisible() const;
    void SetChargeDensityVisible(bool visible);
    void SyncChargeDensityViewTypeState();

    int32_t GetChargeDensityStructureId() const;
    void SetChargeDensityStructureId(int32_t structureId);

    bool IsAdvancedGridVisible(int32_t meshId, bool volumeMode) const;
    void SetAdvancedGridVisible(int32_t meshId, bool volumeMode, bool visible);
    void SetAllAdvancedGridVisible(int32_t structureId, bool volumeMode, bool visible);
    void ApplyAdvancedGridVisibilityForStructure(int32_t structureId);

    atoms::ui::ChargeDensityUI* ChargeDensityUi();
    const atoms::ui::ChargeDensityUI* ChargeDensityUi() const;

private:
    AtomsTemplate* m_atomsTemplate = nullptr;
};

} // namespace application
} // namespace density

