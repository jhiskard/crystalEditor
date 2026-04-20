/**
 * @file legacy_measurement_service_port.h
 * @brief Legacy measurement adapter implementing MeasurementServicePort.
 * @note Temporary compatibility path (remove in Phase18-W6, Track: P18-W3-MEASUREMENT-PORT).
 */
#pragma once

#include "../../application/measurement_service_port.h"

namespace measurement {
namespace infrastructure {
namespace legacy {

/**
 * @brief Adapts measurement service port calls to legacy AtomsTemplate facade.
 */
class LegacyMeasurementServicePort final : public application::MeasurementServicePort {
public:
    application::MeasurementMode GetMode() const override;
    bool IsModeActive() const override;
    void EnterMode(application::MeasurementMode mode) override;
    void ExitMode() override;

    std::vector<application::MeasurementListItem> GetMeasurementsForStructure(int32_t structureId) const override;
    void SetMeasurementVisible(uint32_t measurementId, bool visible) override;
    void RemoveMeasurement(uint32_t measurementId) override;
    void RemoveMeasurementsByStructure(int32_t structureId) override;

    std::vector<application::DistanceMeasurementListItem> GetDistanceMeasurementsForStructure(
        int32_t structureId) const override;
    void SetDistanceMeasurementVisible(uint32_t measurementId, bool visible) override;
    void RemoveDistanceMeasurement(uint32_t measurementId) override;
};

} // namespace legacy
} // namespace infrastructure
} // namespace measurement
