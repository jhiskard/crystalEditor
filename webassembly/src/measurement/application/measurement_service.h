/**
 * @file measurement_service.h
 * @brief Measurement feature application service facade.
 */
#pragma once

#include "measurement_service_port.h"
#include "measurement_service_types.h"

#include <cstdint>
#include <vector>

namespace measurement {
namespace application {

/**
 * @brief Measurement use-case facade extracted from AtomsTemplate.
 * @details 서비스 레이어는 포트 계약만 의존하며, legacy 호환 경로는
 *          infrastructure adapter에서만 관리한다.
 */
class MeasurementService {
public:
    /**
     * @brief Creates measurement service with default legacy adapter.
     */
    MeasurementService();

    /**
     * @brief Creates measurement service with injected compatibility port.
     */
    explicit MeasurementService(MeasurementServicePort& port);

    MeasurementMode GetMode() const;
    bool IsModeActive() const;
    void EnterMode(MeasurementMode mode);
    void ExitMode();

    std::vector<MeasurementListItem> GetMeasurementsForStructure(int32_t structureId) const;
    void SetMeasurementVisible(uint32_t measurementId, bool visible);
    void RemoveMeasurement(uint32_t measurementId);
    void RemoveMeasurementsByStructure(int32_t structureId);

    std::vector<DistanceMeasurementListItem> GetDistanceMeasurementsForStructure(int32_t structureId) const;
    void SetDistanceMeasurementVisible(uint32_t measurementId, bool visible);
    void RemoveDistanceMeasurement(uint32_t measurementId);

private:
    MeasurementServicePort* m_Port { nullptr };
};

} // namespace application
} // namespace measurement

