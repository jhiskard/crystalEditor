/**
 * @file measurement_service.h
 * @brief Measurement feature application service facade.
 */
#pragma once

#include "measurement_service_types.h"

#include <cstdint>
#include <vector>

namespace measurement {
namespace application {

/**
 * @brief Measurement use-case facade extracted from AtomsTemplate.
 * @details R6 단계에서 compatibility port 의존을 제거하고 서비스가 직접
 *          legacy atoms runtime 경로를 호출하도록 전환했다.
 */
class MeasurementService {
public:
    /**
     * @brief Creates measurement service.
     */
    MeasurementService() = default;

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

};

} // namespace application
} // namespace measurement

