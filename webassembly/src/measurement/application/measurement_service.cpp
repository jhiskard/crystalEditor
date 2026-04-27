#include "measurement_service.h"

#include "../../workspace/runtime/legacy_atoms_runtime.h"

namespace {
measurement::application::MeasurementMode toMeasurementMode(WorkspaceRuntimeModel::MeasurementMode mode) {
    using measurement::application::MeasurementMode;
    switch (mode) {
    case WorkspaceRuntimeModel::MeasurementMode::Distance:
        return MeasurementMode::Distance;
    case WorkspaceRuntimeModel::MeasurementMode::Angle:
        return MeasurementMode::Angle;
    case WorkspaceRuntimeModel::MeasurementMode::Dihedral:
        return MeasurementMode::Dihedral;
    case WorkspaceRuntimeModel::MeasurementMode::GeometricCenter:
        return MeasurementMode::GeometricCenter;
    case WorkspaceRuntimeModel::MeasurementMode::CenterOfMass:
        return MeasurementMode::CenterOfMass;
    case WorkspaceRuntimeModel::MeasurementMode::None:
    default:
        return MeasurementMode::None;
    }
}

WorkspaceRuntimeModel::MeasurementMode toLegacyMeasurementMode(measurement::application::MeasurementMode mode) {
    using measurement::application::MeasurementMode;
    switch (mode) {
    case MeasurementMode::Distance:
        return WorkspaceRuntimeModel::MeasurementMode::Distance;
    case MeasurementMode::Angle:
        return WorkspaceRuntimeModel::MeasurementMode::Angle;
    case MeasurementMode::Dihedral:
        return WorkspaceRuntimeModel::MeasurementMode::Dihedral;
    case MeasurementMode::GeometricCenter:
        return WorkspaceRuntimeModel::MeasurementMode::GeometricCenter;
    case MeasurementMode::CenterOfMass:
        return WorkspaceRuntimeModel::MeasurementMode::CenterOfMass;
    case MeasurementMode::None:
    default:
        return WorkspaceRuntimeModel::MeasurementMode::None;
    }
}

measurement::application::MeasurementType toMeasurementType(WorkspaceRuntimeModel::MeasurementType type) {
    using measurement::application::MeasurementType;
    switch (type) {
    case WorkspaceRuntimeModel::MeasurementType::Distance:
        return MeasurementType::Distance;
    case WorkspaceRuntimeModel::MeasurementType::Angle:
        return MeasurementType::Angle;
    case WorkspaceRuntimeModel::MeasurementType::Dihedral:
        return MeasurementType::Dihedral;
    case WorkspaceRuntimeModel::MeasurementType::GeometricCenter:
        return MeasurementType::GeometricCenter;
    case WorkspaceRuntimeModel::MeasurementType::CenterOfMass:
    default:
        return MeasurementType::CenterOfMass;
    }
}
} // namespace

namespace measurement {
namespace application {

MeasurementService::MeasurementService() = default;

MeasurementMode MeasurementService::GetMode() const {
    return toMeasurementMode(workspace::legacy::WorkspaceRuntimeModelRef().GetMeasurementMode());
}

bool MeasurementService::IsModeActive() const {
    return workspace::legacy::WorkspaceRuntimeModelRef().IsMeasurementModeActive();
}

bool MeasurementService::IsDragSelectionEnabled() const {
    return workspace::legacy::WorkspaceRuntimeModelRef().IsMeasurementDragSelectionEnabled();
}

void MeasurementService::EnterMode(MeasurementMode mode) {
    workspace::legacy::WorkspaceRuntimeModelRef().EnterMeasurementMode(toLegacyMeasurementMode(mode));
}

void MeasurementService::ExitMode() {
    workspace::legacy::WorkspaceRuntimeModelRef().ExitMeasurementMode();
}

render::application::PickedAtomInfo MeasurementService::ResolvePickedAtomInfo(
    vtkActor* actor,
    const double pickPos[3]) const {
    return workspace::legacy::WorkspaceRuntimeModelRef().ResolvePickedAtomInfo(actor, pickPos);
}

void MeasurementService::HandlePickerClick(const render::application::PickedAtomInfo& pickedAtomInfo) {
    workspace::legacy::WorkspaceRuntimeModelRef().HandleMeasurementClickByPicker(pickedAtomInfo);
}

void MeasurementService::HandleEmptyClick() {
    workspace::legacy::WorkspaceRuntimeModelRef().HandleMeasurementEmptyClick();
}

void MeasurementService::HandleDragSelectionInScreenRect(
    int x0,
    int y0,
    int x1,
    int y1,
    vtkRenderer* renderer,
    int viewportHeight,
    bool additive) {
    workspace::legacy::WorkspaceRuntimeModelRef().HandleDragSelectionInScreenRect(
        x0, y0, x1, y1, renderer, viewportHeight, additive);
}

void MeasurementService::RenderModeOverlay() {
    workspace::legacy::WorkspaceRuntimeModelRef().RenderMeasurementModeOverlay();
}

std::vector<MeasurementListItem> MeasurementService::GetMeasurementsForStructure(int32_t structureId) const {
    std::vector<MeasurementListItem> output;
    const auto source = workspace::legacy::WorkspaceRuntimeModelRef().GetMeasurementsForStructure(structureId);
    output.reserve(source.size());
    for (const auto& item : source) {
        output.push_back({ item.id, toMeasurementType(item.type), item.displayName, item.visible });
    }
    return output;
}

void MeasurementService::SetMeasurementVisible(uint32_t measurementId, bool visible) {
    workspace::legacy::WorkspaceRuntimeModelRef().SetMeasurementVisible(measurementId, visible);
}

void MeasurementService::RemoveMeasurement(uint32_t measurementId) {
    workspace::legacy::WorkspaceRuntimeModelRef().RemoveMeasurement(measurementId);
}

void MeasurementService::RemoveMeasurementsByStructure(int32_t structureId) {
    workspace::legacy::WorkspaceRuntimeModelRef().RemoveMeasurementsByStructure(structureId);
}

std::vector<DistanceMeasurementListItem> MeasurementService::GetDistanceMeasurementsForStructure(
    int32_t structureId) const {
    std::vector<DistanceMeasurementListItem> output;
    const auto source = workspace::legacy::WorkspaceRuntimeModelRef().GetDistanceMeasurementsForStructure(structureId);
    output.reserve(source.size());
    for (const auto& item : source) {
        output.push_back({ item.id, item.displayName, item.visible });
    }
    return output;
}

void MeasurementService::SetDistanceMeasurementVisible(uint32_t measurementId, bool visible) {
    workspace::legacy::WorkspaceRuntimeModelRef().SetDistanceMeasurementVisible(measurementId, visible);
}

void MeasurementService::RemoveDistanceMeasurement(uint32_t measurementId) {
    workspace::legacy::WorkspaceRuntimeModelRef().RemoveDistanceMeasurement(measurementId);
}

} // namespace application
} // namespace measurement
