/**
 * @file measurement_service_legacy_port_adapter.h
 * @brief Legacy adapter that bridges MeasurementServicePort to AtomsTemplate.
 */
#pragma once

#include "../../measurement/application/measurement_service_port.h"

class AtomsTemplate;

namespace atoms {
namespace application {

/**
 * @brief Measurement service port adapter backed by AtomsTemplate.
 */
class MeasurementServiceLegacyPortAdapter final : public measurement::application::MeasurementServicePort {
public:
    explicit MeasurementServiceLegacyPortAdapter(AtomsTemplate* atomsTemplate);

    measurement::application::MeasurementMode GetMode() const override;
    bool IsModeActive() const override;
    void EnterMode(measurement::application::MeasurementMode mode) override;
    void ExitMode() override;

    std::vector<measurement::application::MeasurementListItem>
    GetMeasurementsForStructure(int32_t structureId) const override;
    void SetMeasurementVisible(uint32_t measurementId, bool visible) override;
    void RemoveMeasurement(uint32_t measurementId) override;
    void RemoveMeasurementsByStructure(int32_t structureId) override;

    std::vector<measurement::application::DistanceMeasurementListItem>
    GetDistanceMeasurementsForStructure(int32_t structureId) const override;
    void SetDistanceMeasurementVisible(uint32_t measurementId, bool visible) override;
    void RemoveDistanceMeasurement(uint32_t measurementId) override;

private:
    AtomsTemplate* m_atomsTemplate = nullptr;
};

} // namespace application
} // namespace atoms

