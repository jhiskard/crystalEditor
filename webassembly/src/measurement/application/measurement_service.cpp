#include "measurement_service.h"

namespace measurement {
namespace application {

MeasurementService::MeasurementService(MeasurementServicePort* port)
    : m_port(port) {}

MeasurementMode MeasurementService::GetMode() const {
    if (!m_port) {
        return MeasurementMode::None;
    }
    return m_port->GetMode();
}

bool MeasurementService::IsModeActive() const {
    if (!m_port) {
        return false;
    }
    return m_port->IsModeActive();
}

void MeasurementService::EnterMode(MeasurementMode mode) {
    if (!m_port) {
        return;
    }
    m_port->EnterMode(mode);
}

void MeasurementService::ExitMode() {
    if (!m_port) {
        return;
    }
    m_port->ExitMode();
}

std::vector<MeasurementListItem> MeasurementService::GetMeasurementsForStructure(int32_t structureId) const {
    if (!m_port) {
        return {};
    }
    return m_port->GetMeasurementsForStructure(structureId);
}

void MeasurementService::SetMeasurementVisible(uint32_t measurementId, bool visible) {
    if (!m_port) {
        return;
    }
    m_port->SetMeasurementVisible(measurementId, visible);
}

void MeasurementService::RemoveMeasurement(uint32_t measurementId) {
    if (!m_port) {
        return;
    }
    m_port->RemoveMeasurement(measurementId);
}

void MeasurementService::RemoveMeasurementsByStructure(int32_t structureId) {
    if (!m_port) {
        return;
    }
    m_port->RemoveMeasurementsByStructure(structureId);
}

std::vector<DistanceMeasurementListItem> MeasurementService::GetDistanceMeasurementsForStructure(
    int32_t structureId) const {
    if (!m_port) {
        return {};
    }
    return m_port->GetDistanceMeasurementsForStructure(structureId);
}

void MeasurementService::SetDistanceMeasurementVisible(uint32_t measurementId, bool visible) {
    if (!m_port) {
        return;
    }
    m_port->SetDistanceMeasurementVisible(measurementId, visible);
}

void MeasurementService::RemoveDistanceMeasurement(uint32_t measurementId) {
    if (!m_port) {
        return;
    }
    m_port->RemoveDistanceMeasurement(measurementId);
}

} // namespace application
} // namespace measurement

