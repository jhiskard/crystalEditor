#include "measurement_service.h"

#include "../../workspace/legacy/atoms_template_facade.h"

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

MeasurementMode MeasurementService::GetMode() const {
    return ToMeasurementMode(AtomsTemplate::Instance().GetMeasurementMode());
}

bool MeasurementService::IsModeActive() const {
    return AtomsTemplate::Instance().IsMeasurementModeActive();
}

void MeasurementService::EnterMode(MeasurementMode mode) {
    AtomsTemplate::Instance().EnterMeasurementMode(ToLegacyMeasurementMode(mode));
}

void MeasurementService::ExitMode() {
    AtomsTemplate::Instance().ExitMeasurementMode();
}

std::vector<MeasurementListItem> MeasurementService::GetMeasurementsForStructure(int32_t structureId) const {
    std::vector<MeasurementListItem> output;
    const auto source = AtomsTemplate::Instance().GetMeasurementsForStructure(structureId);
    output.reserve(source.size());
    for (const auto& item : source) {
        output.push_back({item.id, item.displayName, item.visible});
    }
    return output;
}

void MeasurementService::SetMeasurementVisible(uint32_t measurementId, bool visible) {
    AtomsTemplate::Instance().SetMeasurementVisible(measurementId, visible);
}

void MeasurementService::RemoveMeasurement(uint32_t measurementId) {
    AtomsTemplate::Instance().RemoveMeasurement(measurementId);
}

void MeasurementService::RemoveMeasurementsByStructure(int32_t structureId) {
    AtomsTemplate::Instance().RemoveMeasurementsByStructure(structureId);
}

std::vector<DistanceMeasurementListItem> MeasurementService::GetDistanceMeasurementsForStructure(
    int32_t structureId) const {
    std::vector<DistanceMeasurementListItem> output;
    const auto source = AtomsTemplate::Instance().GetDistanceMeasurementsForStructure(structureId);
    output.reserve(source.size());
    for (const auto& item : source) {
        output.push_back({item.id, item.displayName, item.visible});
    }
    return output;
}

void MeasurementService::SetDistanceMeasurementVisible(uint32_t measurementId, bool visible) {
    AtomsTemplate::Instance().SetDistanceMeasurementVisible(measurementId, visible);
}

void MeasurementService::RemoveDistanceMeasurement(uint32_t measurementId) {
    AtomsTemplate::Instance().RemoveDistanceMeasurement(measurementId);
}

} // namespace application
} // namespace measurement
