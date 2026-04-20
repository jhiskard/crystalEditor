#include "workbench_controller.h"

#include "shell_state_command_service.h"
#include "shell_state_query_service.h"
#include "../../workspace/legacy/legacy_atoms_runtime.h"
#include "../../mesh/application/mesh_command_service.h"
#include "../../render/application/render_gateway.h"
#include "../../structure/application/structure_service.h"
#include "../runtime/workbench_runtime.h"

#include <algorithm>

namespace {

void requestFocus(shell::domain::ShellFocusTarget target, int passes = 2) {
    shell::application::GetShellStateCommandService().RequestFocus(target, passes);
}

shell::domain::ShellUiState& mutableShellState() {
    return shell::application::GetShellStateCommandService().MutableState();
}

void consumeInitialLayoutBootstrap(shell::domain::ShellUiState& state) {
    state.shouldApplyInitialLayout = false;
}

measurement::application::MeasurementService& measurementService() {
    return GetWorkbenchRuntime().MeasurementFeature();
}

structure::application::StructureService& structureService() {
    return GetWorkbenchRuntime().StructureFeature();
}

} // namespace

namespace shell {
namespace application {

WorkbenchController& WorkbenchController::Instance() {
    static WorkbenchController controller;
    return controller;
}

void WorkbenchController::RequestOpenStructureImport() {
    GetWorkbenchRuntime().RequestOpenStructureImport();
}

void WorkbenchController::OpenEditorPanel(EditorPanelAction action) {
    domain::ShellUiState& state = mutableShellState();
    consumeInitialLayoutBootstrap(state);
    ShellStateCommandService& shellCommand = GetShellStateCommandService();

    switch (action) {
    case EditorPanelAction::Atoms:
        state.showCreatedAtomsWindow = true;
        requestFocus(domain::ShellFocusTarget::CreatedAtoms);
        shellCommand.RequestEditorPanel(workbench::panel::EditorRequest::Atoms);
        break;
    case EditorPanelAction::Bonds:
        state.showBondsManagementWindow = true;
        requestFocus(domain::ShellFocusTarget::BondsManagement);
        shellCommand.RequestEditorPanel(workbench::panel::EditorRequest::Bonds);
        break;
    case EditorPanelAction::Cell:
        state.showCellInformationWindow = true;
        requestFocus(domain::ShellFocusTarget::CellInformation);
        shellCommand.RequestEditorPanel(workbench::panel::EditorRequest::Cell);
        break;
    }
}

void WorkbenchController::OpenBuilderPanel(BuilderPanelAction action) {
    domain::ShellUiState& state = mutableShellState();
    consumeInitialLayoutBootstrap(state);
    ShellStateCommandService& shellCommand = GetShellStateCommandService();

    switch (action) {
    case BuilderPanelAction::AddAtoms:
        state.showPeriodicTableWindow = true;
        requestFocus(domain::ShellFocusTarget::PeriodicTable);
        shellCommand.RequestBuilderPanel(workbench::panel::BuilderRequest::AddAtoms);
        break;
    case BuilderPanelAction::BravaisLatticeTemplates:
        state.showCrystalTemplatesWindow = true;
        requestFocus(domain::ShellFocusTarget::CrystalTemplates);
        shellCommand.RequestBuilderPanel(
            workbench::panel::BuilderRequest::BravaisLatticeTemplates);
        break;
    case BuilderPanelAction::BrillouinZone:
        state.showBrillouinZonePlotWindow = true;
        requestFocus(domain::ShellFocusTarget::BrillouinZonePlot);
        shellCommand.RequestBuilderPanel(
            workbench::panel::BuilderRequest::BrillouinZone);
        break;
    }
}

void WorkbenchController::OpenDataPanel(DataPanelAction action) {
    domain::ShellUiState& state = mutableShellState();
    consumeInitialLayoutBootstrap(state);
    ShellStateCommandService& shellCommand = GetShellStateCommandService();

    state.showModelTree = true;

    switch (action) {
    case DataPanelAction::Isosurface:
        state.showChargeDensityViewerWindow = true;
        requestFocus(domain::ShellFocusTarget::ChargeDensityViewer);
        shellCommand.RequestDataPanel(workbench::panel::DataRequest::Isosurface);
        break;
    case DataPanelAction::Surface:
        state.showChargeDensityViewerWindow = true;
        requestFocus(domain::ShellFocusTarget::ChargeDensityViewer);
        shellCommand.RequestDataPanel(workbench::panel::DataRequest::Surface);
        break;
    case DataPanelAction::Volumetric:
        state.showChargeDensityViewerWindow = true;
        requestFocus(domain::ShellFocusTarget::ChargeDensityViewer);
        shellCommand.RequestDataPanel(workbench::panel::DataRequest::Volumetric);
        break;
    case DataPanelAction::Plane:
        state.showSliceViewerWindow = true;
        requestFocus(domain::ShellFocusTarget::SliceViewer);
        shellCommand.RequestDataPanel(workbench::panel::DataRequest::Plane);
        break;
    }
}

bool WorkbenchController::IsNodeInfoEnabled() const {
    return workspace::legacy::LegacyAtomsRuntime().IsNodeInfoEnabled();
}

void WorkbenchController::SetNodeInfoEnabled(bool enabled) {
    workspace::legacy::LegacyAtomsRuntime().SetNodeInfoEnabled(enabled);
}

bool WorkbenchController::IsViewerFpsOverlayEnabled() const {
    return render::application::GetRenderGateway().IsPerformanceOverlayEnabled();
}

void WorkbenchController::SetViewerFpsOverlayEnabled(bool enabled) {
    render::application::GetRenderGateway().SetPerformanceOverlayEnabled(enabled);
}

bool WorkbenchController::IsMeasurementModeActive(measurement::application::MeasurementMode mode) const {
    return measurementService().GetMode() == mode;
}

void WorkbenchController::EnterMeasurementMode(measurement::application::MeasurementMode mode) {
    measurementService().EnterMode(mode);
}

void WorkbenchController::RequestLayoutPreset(domain::ShellLayoutPreset preset) {
    domain::ShellUiState& state = mutableShellState();
    consumeInitialLayoutBootstrap(state);
    state.pendingLayoutPreset = preset;
}

void WorkbenchController::ToggleBoundaryAtoms() {
    structure::application::StructureService& feature = structureService();
    feature.SetBoundaryAtomsEnabled(!feature.IsBoundaryAtomsEnabled());
}

bool WorkbenchController::IsBoundaryAtomsEnabled() const {
    return structureService().IsBoundaryAtomsEnabled();
}

void WorkbenchController::SetMeshDisplayMode(MeshDisplayMode mode) {
    mesh::application::GetMeshCommandService().SetAllDisplayMode(mode);
}

void WorkbenchController::SetProjectionMode(ProjectionMode mode) {
    render::application::GetRenderGateway().SetProjectionMode(mode);
}

void WorkbenchController::ResetView() {
    render::application::GetRenderGateway().ResetView();
}

void WorkbenchController::AlignCameraToActiveCellAxis(int axisIndex) {
    const bool isBzMode = GetWorkbenchRuntime().ShellStateQuery().IsWindowVisible(
        domain::ShellWindowId::BrillouinZonePlot);
    if (isBzMode) {
        render::application::GetRenderGateway().AlignCameraToIcellAxis(axisIndex);
        return;
    }

    render::application::GetRenderGateway().AlignCameraToCellAxis(axisIndex);
}

int WorkbenchController::GetArrowRotateStepDeg() const {
    return static_cast<int>(render::application::GetRenderGateway().GetArrowRotateStepDeg());
}

void WorkbenchController::SetArrowRotateStepDeg(int stepDeg) {
    const int clamped = std::clamp(stepDeg, 1, 180);
    render::application::GetRenderGateway().SetArrowRotateStepDeg(static_cast<float>(clamped));
}

WorkbenchController& GetWorkbenchController() {
    return WorkbenchController::Instance();
}

} // namespace application
} // namespace shell




