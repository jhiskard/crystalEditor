/**
 * @file measurement_service_port.h
 * @brief Port interface for measurement service compatibility backends.
 */
#pragma once

#include "measurement_service_types.h"

#include <cstdint>
#include <vector>

namespace measurement {
namespace application {

/**
 * @brief Port abstraction used by MeasurementService.
 */
class MeasurementServicePort {
public:
    virtual ~MeasurementServicePort() = default;

    virtual MeasurementMode GetMode() const = 0;
    virtual bool IsModeActive() const = 0;
    virtual void EnterMode(MeasurementMode mode) = 0;
    virtual void ExitMode() = 0;

    virtual std::vector<MeasurementListItem> GetMeasurementsForStructure(int32_t structureId) const = 0;
    virtual void SetMeasurementVisible(uint32_t measurementId, bool visible) = 0;
    virtual void RemoveMeasurement(uint32_t measurementId) = 0;
    virtual void RemoveMeasurementsByStructure(int32_t structureId) = 0;

    virtual std::vector<DistanceMeasurementListItem> GetDistanceMeasurementsForStructure(int32_t structureId) const = 0;
    virtual void SetDistanceMeasurementVisible(uint32_t measurementId, bool visible) = 0;
    virtual void RemoveDistanceMeasurement(uint32_t measurementId) = 0;
};

} // namespace application
} // namespace measurement

