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
 * @details During Phase 8 this service delegates to a compatibility port so call-sites
 *          can migrate without behavior changes.
 */
class MeasurementService {
public:
    /**
     * @brief Creates service bound to the measurement compatibility port.
     */
    explicit MeasurementService(MeasurementServicePort* port);

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
    MeasurementServicePort* m_port = nullptr;
};

} // namespace application
} // namespace measurement

