#include "measurement_service.h"

#include "../../atoms/atoms_template.h"

namespace {
measurement::application::MeasurementMode ToMeasurementMode(AtomsTemplate::MeasurementMode mode) {
    switch (mode) {
    case AtomsTemplate::MeasurementMode::Distance:
        return measurement::application::MeasurementMode::Distance;
    case AtomsTemplate::MeasurementMode::Angle:
        return measurement::application::MeasurementMode::Angle;
    case AtomsTemplate::MeasurementMode::Dihedral:
        return measurement::application::MeasurementMode::Dihedral;
    case AtomsTemplate::MeasurementMode::GeometricCenter:
        return measurement::application::MeasurementMode::GeometricCenter;
    case AtomsTemplate::MeasurementMode::CenterOfMass:
        return measurement::application::MeasurementMode::CenterOfMass;
    case AtomsTemplate::MeasurementMode::None:
    default:
        return measurement::application::MeasurementMode::None;
    }
}

AtomsTemplate::MeasurementMode ToLegacyMeasurementMode(measurement::application::MeasurementMode mode) {
    switch (mode) {
    case measurement::application::MeasurementMode::Distance:
        return AtomsTemplate::MeasurementMode::Distance;
    case measurement::application::MeasurementMode::Angle:
        return AtomsTemplate::MeasurementMode::Angle;
    case measurement::application::MeasurementMode::Dihedral:
        return AtomsTemplate::MeasurementMode::Dihedral;
    case measurement::application::MeasurementMode::GeometricCenter:
        return AtomsTemplate::MeasurementMode::GeometricCenter;
    case measurement::application::MeasurementMode::CenterOfMass:
        return AtomsTemplate::MeasurementMode::CenterOfMass;
    case measurement::application::MeasurementMode::None:
    default:
        return AtomsTemplate::MeasurementMode::None;
    }
}
} // namespace

namespace measurement {
namespace application {

MeasurementService::MeasurementService(AtomsTemplate* atomsTemplate)
    : m_atomsTemplate(atomsTemplate) {}

MeasurementMode MeasurementService::GetMode() const {
    if (!m_atomsTemplate) {
        return MeasurementMode::None;
    }
    return ToMeasurementMode(m_atomsTemplate->GetMeasurementMode());
}

bool MeasurementService::IsModeActive() const {
    if (!m_atomsTemplate) {
        return false;
    }
    return m_atomsTemplate->IsMeasurementModeActive();
}

void MeasurementService::EnterMode(MeasurementMode mode) {
    if (!m_atomsTemplate) {
        return;
    }
    m_atomsTemplate->EnterMeasurementMode(ToLegacyMeasurementMode(mode));
}

void MeasurementService::ExitMode() {
    if (!m_atomsTemplate) {
        return;
    }
    m_atomsTemplate->ExitMeasurementMode();
}

std::vector<MeasurementListItem> MeasurementService::GetMeasurementsForStructure(int32_t structureId) const {
    std::vector<MeasurementListItem> output;
    if (!m_atomsTemplate) {
        return output;
    }

    const auto source = m_atomsTemplate->GetMeasurementsForStructure(structureId);
    output.reserve(source.size());
    for (const auto& item : source) {
        output.push_back(MeasurementListItem { item.id, item.displayName, item.visible });
    }
    return output;
}

void MeasurementService::SetMeasurementVisible(uint32_t measurementId, bool visible) {
    if (!m_atomsTemplate) {
        return;
    }
    m_atomsTemplate->SetMeasurementVisible(measurementId, visible);
}

void MeasurementService::RemoveMeasurement(uint32_t measurementId) {
    if (!m_atomsTemplate) {
        return;
    }
    m_atomsTemplate->RemoveMeasurement(measurementId);
}

void MeasurementService::RemoveMeasurementsByStructure(int32_t structureId) {
    if (!m_atomsTemplate) {
        return;
    }
    m_atomsTemplate->RemoveMeasurementsByStructure(structureId);
}

std::vector<DistanceMeasurementListItem> MeasurementService::GetDistanceMeasurementsForStructure(
    int32_t structureId) const {
    std::vector<DistanceMeasurementListItem> output;
    if (!m_atomsTemplate) {
        return output;
    }

    const auto source = m_atomsTemplate->GetDistanceMeasurementsForStructure(structureId);
    output.reserve(source.size());
    for (const auto& item : source) {
        output.push_back(DistanceMeasurementListItem { item.id, item.displayName, item.visible });
    }
    return output;
}

void MeasurementService::SetDistanceMeasurementVisible(uint32_t measurementId, bool visible) {
    if (!m_atomsTemplate) {
        return;
    }
    m_atomsTemplate->SetDistanceMeasurementVisible(measurementId, visible);
}

void MeasurementService::RemoveDistanceMeasurement(uint32_t measurementId) {
    if (!m_atomsTemplate) {
        return;
    }
    m_atomsTemplate->RemoveDistanceMeasurement(measurementId);
}

} // namespace application
} // namespace measurement

