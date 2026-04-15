/**
 * @file density_service_legacy_port_adapter.h
 * @brief Legacy adapter that bridges DensityServicePort to AtomsTemplate.
 */
#pragma once

#include "../../density/application/density_service_port.h"

class AtomsTemplate;

namespace atoms {
namespace application {

/**
 * @brief Density service port adapter backed by AtomsTemplate.
 */
class DensityServiceLegacyPortAdapter final : public density::application::DensityServicePort {
public:
    explicit DensityServiceLegacyPortAdapter(AtomsTemplate* atomsTemplate);

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

private:
    AtomsTemplate* m_atomsTemplate = nullptr;
};

} // namespace application
} // namespace atoms

