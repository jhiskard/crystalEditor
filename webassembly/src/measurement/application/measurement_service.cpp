#include "measurement_service.h"

#include "../../workspace/legacy/legacy_atoms_runtime.h"

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

measurement::application::MeasurementType toMeasurementType(AtomsTemplate::MeasurementType type) {
    using measurement::application::MeasurementType;
    switch (type) {
    case AtomsTemplate::MeasurementType::Distance:
        return MeasurementType::Distance;
    case AtomsTemplate::MeasurementType::Angle:
        return MeasurementType::Angle;
    case AtomsTemplate::MeasurementType::Dihedral:
        return MeasurementType::Dihedral;
    case AtomsTemplate::MeasurementType::GeometricCenter:
        return MeasurementType::GeometricCenter;
    case AtomsTemplate::MeasurementType::CenterOfMass:
    default:
        return MeasurementType::CenterOfMass;
    }
}
} // namespace

namespace measurement {
namespace application {

MeasurementService::MeasurementService() = default;

MeasurementMode MeasurementService::GetMode() const {
    return toMeasurementMode(workspace::legacy::LegacyAtomsRuntime().GetMeasurementMode());
}

bool MeasurementService::IsModeActive() const {
    return workspace::legacy::LegacyAtomsRuntime().IsMeasurementModeActive();
}

bool MeasurementService::IsDragSelectionEnabled() const {
    return workspace::legacy::LegacyAtomsRuntime().IsMeasurementDragSelectionEnabled();
}

void MeasurementService::EnterMode(MeasurementMode mode) {
    workspace::legacy::LegacyAtomsRuntime().EnterMeasurementMode(toLegacyMeasurementMode(mode));
}

void MeasurementService::ExitMode() {
    workspace::legacy::LegacyAtomsRuntime().ExitMeasurementMode();
}

render::application::PickedAtomInfo MeasurementService::ResolvePickedAtomInfo(
    vtkActor* actor,
    const double pickPos[3]) const {
    return workspace::legacy::LegacyAtomsRuntime().ResolvePickedAtomInfo(actor, pickPos);
}

void MeasurementService::HandlePickerClick(const render::application::PickedAtomInfo& pickedAtomInfo) {
    workspace::legacy::LegacyAtomsRuntime().HandleMeasurementClickByPicker(pickedAtomInfo);
}

void MeasurementService::HandleEmptyClick() {
    workspace::legacy::LegacyAtomsRuntime().HandleMeasurementEmptyClick();
}

void MeasurementService::HandleDragSelectionInScreenRect(
    int x0,
    int y0,
    int x1,
    int y1,
    vtkRenderer* renderer,
    int viewportHeight,
    bool additive) {
    workspace::legacy::LegacyAtomsRuntime().HandleDragSelectionInScreenRect(
        x0, y0, x1, y1, renderer, viewportHeight, additive);
}

void MeasurementService::RenderModeOverlay() {
    workspace::legacy::LegacyAtomsRuntime().RenderMeasurementModeOverlay();
}

std::vector<MeasurementListItem> MeasurementService::GetMeasurementsForStructure(int32_t structureId) const {
    std::vector<MeasurementListItem> output;
    const auto source = workspace::legacy::LegacyAtomsRuntime().GetMeasurementsForStructure(structureId);
    output.reserve(source.size());
    for (const auto& item : source) {
        output.push_back({ item.id, toMeasurementType(item.type), item.displayName, item.visible });
    }
    return output;
}

void MeasurementService::SetMeasurementVisible(uint32_t measurementId, bool visible) {
    workspace::legacy::LegacyAtomsRuntime().SetMeasurementVisible(measurementId, visible);
}

void MeasurementService::RemoveMeasurement(uint32_t measurementId) {
    workspace::legacy::LegacyAtomsRuntime().RemoveMeasurement(measurementId);
}

void MeasurementService::RemoveMeasurementsByStructure(int32_t structureId) {
    workspace::legacy::LegacyAtomsRuntime().RemoveMeasurementsByStructure(structureId);
}

std::vector<DistanceMeasurementListItem> MeasurementService::GetDistanceMeasurementsForStructure(
    int32_t structureId) const {
    std::vector<DistanceMeasurementListItem> output;
    const auto source = workspace::legacy::LegacyAtomsRuntime().GetDistanceMeasurementsForStructure(structureId);
    output.reserve(source.size());
    for (const auto& item : source) {
        output.push_back({ item.id, item.displayName, item.visible });
    }
    return output;
}

void MeasurementService::SetDistanceMeasurementVisible(uint32_t measurementId, bool visible) {
    workspace::legacy::LegacyAtomsRuntime().SetDistanceMeasurementVisible(measurementId, visible);
}

void MeasurementService::RemoveDistanceMeasurement(uint32_t measurementId) {
    workspace::legacy::LegacyAtomsRuntime().RemoveDistanceMeasurement(measurementId);
}

} // namespace application
} // namespace measurement
