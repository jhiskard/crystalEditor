/**
 * @file density_service.h
 * @brief Charge-density feature application service facade.
 */
#pragma once

#include "density_service_port.h"

#include <cstdint>

namespace density {
namespace application {

/**
 * @brief Density use-case facade extracted from AtomsTemplate.
 * @details 서비스 레이어는 포트 계약만 의존하며, legacy 호환 경로는
 *          infrastructure adapter에서만 관리한다.
 */
class DensityService {
public:
    /**
     * @brief Creates density service with default legacy adapter.
     */
    DensityService();

    /**
     * @brief Creates density service with injected compatibility port.
     */
    explicit DensityService(DensityServicePort& port);

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

private:
    DensityServicePort* m_Port { nullptr };
};

} // namespace application
} // namespace density

