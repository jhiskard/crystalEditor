#include "legacy_measurement_service_port.h"

#include "../../../workspace/legacy/legacy_atoms_runtime.h"

namespace {
measurement::application::MeasurementMode toMeasurementMode(AtomsTemplate::MeasurementMode mode) {
    using measurement::application::MeasurementMode;
    switch (mode) {
    case AtomsTemplate::MeasurementMode::Distance:
        return MeasurementMode::Distance;
    case AtomsTemplate::MeasurementMode::Angle:
        return MeasurementMode::Angle;
    case AtomsTemplate::MeasurementMode::Dihedral:
        return MeasurementMode::Dihedral;
    case AtomsTemplate::MeasurementMode::GeometricCenter:
        return MeasurementMode::GeometricCenter;
    case AtomsTemplate::MeasurementMode::CenterOfMass:
        return MeasurementMode::CenterOfMass;
    case AtomsTemplate::MeasurementMode::None:
    default:
        return MeasurementMode::None;
    }
}

AtomsTemplate::MeasurementMode toLegacyMeasurementMode(measurement::application::MeasurementMode mode) {
    using measurement::application::MeasurementMode;
    switch (mode) {
    case MeasurementMode::Distance:
        return AtomsTemplate::MeasurementMode::Distance;
    case MeasurementMode::Angle:
        return AtomsTemplate::MeasurementMode::Angle;
    case MeasurementMode::Dihedral:
        return AtomsTemplate::MeasurementMode::Dihedral;
    case MeasurementMode::GeometricCenter:
        return AtomsTemplate::MeasurementMode::GeometricCenter;
    case MeasurementMode::CenterOfMass:
        return AtomsTemplate::MeasurementMode::CenterOfMass;
    case MeasurementMode::None:
    default:
        return AtomsTemplate::MeasurementMode::None;
    }
}
} // namespace

namespace measurement {
namespace infrastructure {
namespace legacy {

application::MeasurementMode LegacyMeasurementServicePort::GetMode() const {
    return toMeasurementMode(workspace::legacy::LegacyAtomsRuntime().GetMeasurementMode());
}

bool LegacyMeasurementServicePort::IsModeActive() const {
    return workspace::legacy::LegacyAtomsRuntime().IsMeasurementModeActive();
}

void LegacyMeasurementServicePort::EnterMode(application::MeasurementMode mode) {
    workspace::legacy::LegacyAtomsRuntime().EnterMeasurementMode(toLegacyMeasurementMode(mode));
}

void LegacyMeasurementServicePort::ExitMode() {
    workspace::legacy::LegacyAtomsRuntime().ExitMeasurementMode();
}

std::vector<application::MeasurementListItem> LegacyMeasurementServicePort::GetMeasurementsForStructure(
    int32_t structureId) const {
    std::vector<application::MeasurementListItem> output;
    const auto source = workspace::legacy::LegacyAtomsRuntime().GetMeasurementsForStructure(structureId);
    output.reserve(source.size());
    for (const auto& item : source) {
        output.push_back({item.id, item.displayName, item.visible});
    }
    return output;
}

void LegacyMeasurementServicePort::SetMeasurementVisible(uint32_t measurementId, bool visible) {
    workspace::legacy::LegacyAtomsRuntime().SetMeasurementVisible(measurementId, visible);
}

void LegacyMeasurementServicePort::RemoveMeasurement(uint32_t measurementId) {
    workspace::legacy::LegacyAtomsRuntime().RemoveMeasurement(measurementId);
}

void LegacyMeasurementServicePort::RemoveMeasurementsByStructure(int32_t structureId) {
    workspace::legacy::LegacyAtomsRuntime().RemoveMeasurementsByStructure(structureId);
}

std::vector<application::DistanceMeasurementListItem>
LegacyMeasurementServicePort::GetDistanceMeasurementsForStructure(int32_t structureId) const {
    std::vector<application::DistanceMeasurementListItem> output;
    const auto source = workspace::legacy::LegacyAtomsRuntime().GetDistanceMeasurementsForStructure(structureId);
    output.reserve(source.size());
    for (const auto& item : source) {
        output.push_back({item.id, item.displayName, item.visible});
    }
    return output;
}

void LegacyMeasurementServicePort::SetDistanceMeasurementVisible(uint32_t measurementId, bool visible) {
    workspace::legacy::LegacyAtomsRuntime().SetDistanceMeasurementVisible(measurementId, visible);
}

void LegacyMeasurementServicePort::RemoveDistanceMeasurement(uint32_t measurementId) {
    workspace::legacy::LegacyAtomsRuntime().RemoveDistanceMeasurement(measurementId);
}

} // namespace legacy
} // namespace infrastructure
} // namespace measurement


