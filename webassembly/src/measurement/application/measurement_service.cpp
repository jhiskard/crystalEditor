#include "measurement_service.h"

#include "../infrastructure/legacy/legacy_measurement_service_port.h"

namespace {
measurement::application::MeasurementServicePort& defaultMeasurementServicePort() {
    static measurement::infrastructure::legacy::LegacyMeasurementServicePort port;
    return port;
}
} // namespace

namespace measurement {
namespace application {

MeasurementService::MeasurementService()
    : MeasurementService(defaultMeasurementServicePort()) {}

MeasurementService::MeasurementService(MeasurementServicePort& port)
    : m_Port(&port) {}

MeasurementMode MeasurementService::GetMode() const {
    return m_Port->GetMode();
}

bool MeasurementService::IsModeActive() const {
    return m_Port->IsModeActive();
}

void MeasurementService::EnterMode(MeasurementMode mode) {
    m_Port->EnterMode(mode);
}

void MeasurementService::ExitMode() {
    m_Port->ExitMode();
}

std::vector<MeasurementListItem> MeasurementService::GetMeasurementsForStructure(int32_t structureId) const {
    return m_Port->GetMeasurementsForStructure(structureId);
}

void MeasurementService::SetMeasurementVisible(uint32_t measurementId, bool visible) {
    m_Port->SetMeasurementVisible(measurementId, visible);
}

void MeasurementService::RemoveMeasurement(uint32_t measurementId) {
    m_Port->RemoveMeasurement(measurementId);
}

void MeasurementService::RemoveMeasurementsByStructure(int32_t structureId) {
    m_Port->RemoveMeasurementsByStructure(structureId);
}

std::vector<DistanceMeasurementListItem> MeasurementService::GetDistanceMeasurementsForStructure(
    int32_t structureId) const {
    return m_Port->GetDistanceMeasurementsForStructure(structureId);
}

void MeasurementService::SetDistanceMeasurementVisible(uint32_t measurementId, bool visible) {
    m_Port->SetDistanceMeasurementVisible(measurementId, visible);
}

void MeasurementService::RemoveDistanceMeasurement(uint32_t measurementId) {
    m_Port->RemoveDistanceMeasurement(measurementId);
}

} // namespace application
} // namespace measurement
