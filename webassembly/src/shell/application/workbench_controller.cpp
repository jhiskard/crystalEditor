#include "workbench_controller.h"

#include "shell_state_command_service.h"
#include "../../atoms/atoms_template.h"
#include "../../mesh_manager.h"
#include "../../render/application/render_gateway.h"
#include "../runtime/workbench_runtime.h"

#include <algorithm>

namespace {

void requestFocus(shell::domain::ShellFocusTarget target, int passes = 2) {
    shell::application::GetShellStateCommandService().RequestFocus(target, passes);
}

shell::domain::ShellUiState& mutableShellState() {
    return shell::application::GetShellStateCommandService().MutableState();
}

measurement::application::MeasurementService& measurementService() {
    return GetWorkbenchRuntime().MeasurementFeature();
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

    switch (action) {
    case EditorPanelAction::Atoms:
        state.showCreatedAtomsWindow = true;
        requestFocus(domain::ShellFocusTarget::CreatedAtoms);
        GetWorkbenchRuntime().AtomsTemplateFacade().RequestEditorSection(AtomsTemplate::EditorSectionRequest::Atoms);
        break;
    case EditorPanelAction::Bonds:
        state.showBondsManagementWindow = true;
        requestFocus(domain::ShellFocusTarget::BondsManagement);
        GetWorkbenchRuntime().AtomsTemplateFacade().RequestEditorSection(AtomsTemplate::EditorSectionRequest::Bonds);
        break;
    case EditorPanelAction::Cell:
        state.showCellInformationWindow = true;
        requestFocus(domain::ShellFocusTarget::CellInformation);
        GetWorkbenchRuntime().AtomsTemplateFacade().RequestEditorSection(AtomsTemplate::EditorSectionRequest::Cell);
        break;
    }
}

void WorkbenchController::OpenBuilderPanel(BuilderPanelAction action) {
    domain::ShellUiState& state = mutableShellState();

    switch (action) {
    case BuilderPanelAction::AddAtoms:
        state.showPeriodicTableWindow = true;
        requestFocus(domain::ShellFocusTarget::PeriodicTable);
        GetWorkbenchRuntime().AtomsTemplateFacade().RequestBuilderSection(AtomsTemplate::BuilderSectionRequest::AddAtoms);
        break;
    case BuilderPanelAction::BravaisLatticeTemplates:
        state.showCrystalTemplatesWindow = true;
        requestFocus(domain::ShellFocusTarget::CrystalTemplates);
        GetWorkbenchRuntime().AtomsTemplateFacade().RequestBuilderSection(
            AtomsTemplate::BuilderSectionRequest::BravaisLatticeTemplates);
        break;
    case BuilderPanelAction::BrillouinZone:
        state.showBrillouinZonePlotWindow = true;
        requestFocus(domain::ShellFocusTarget::BrillouinZonePlot);
        GetWorkbenchRuntime().AtomsTemplateFacade().RequestBuilderSection(
            AtomsTemplate::BuilderSectionRequest::BrillouinZone);
        break;
    }
}

void WorkbenchController::OpenDataPanel(DataPanelAction action) {
    domain::ShellUiState& state = mutableShellState();

    state.showModelTree = true;

    switch (action) {
    case DataPanelAction::Isosurface:
        state.showChargeDensityViewerWindow = true;
        requestFocus(domain::ShellFocusTarget::ChargeDensityViewer);
        GetWorkbenchRuntime().AtomsTemplateFacade().RequestDataMenu(AtomsTemplate::DataMenuRequest::Isosurface);
        break;
    case DataPanelAction::Surface:
        state.showChargeDensityViewerWindow = true;
        requestFocus(domain::ShellFocusTarget::ChargeDensityViewer);
        GetWorkbenchRuntime().AtomsTemplateFacade().RequestDataMenu(AtomsTemplate::DataMenuRequest::Surface);
        break;
    case DataPanelAction::Volumetric:
        state.showChargeDensityViewerWindow = true;
        requestFocus(domain::ShellFocusTarget::ChargeDensityViewer);
        GetWorkbenchRuntime().AtomsTemplateFacade().RequestDataMenu(AtomsTemplate::DataMenuRequest::Volumetric);
        break;
    case DataPanelAction::Plane:
        state.showSliceViewerWindow = true;
        requestFocus(domain::ShellFocusTarget::SliceViewer);
        GetWorkbenchRuntime().AtomsTemplateFacade().RequestDataMenu(AtomsTemplate::DataMenuRequest::Plane);
        break;
    }
}

bool WorkbenchController::IsNodeInfoEnabled() const {
    return GetWorkbenchRuntime().AtomsTemplateFacade().IsNodeInfoEnabled();
}

void WorkbenchController::SetNodeInfoEnabled(bool enabled) {
    GetWorkbenchRuntime().AtomsTemplateFacade().SetNodeInfoEnabled(enabled);
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
    GetShellStateCommandService().RequestLayoutPreset(preset);
}

void WorkbenchController::ToggleBoundaryAtoms() {
    AtomsTemplate& atomsTemplate = GetWorkbenchRuntime().AtomsTemplateFacade();
    atomsTemplate.SetBoundaryAtomsEnabled(!atomsTemplate.isSurroundingsVisible());
}

bool WorkbenchController::IsBoundaryAtomsEnabled() const {
    return GetWorkbenchRuntime().AtomsTemplateFacade().isSurroundingsVisible();
}

void WorkbenchController::SetMeshDisplayMode(MeshDisplayMode mode) {
    GetWorkbenchRuntime().MeshRepository().SetAllDisplayMode(mode);
}

void WorkbenchController::SetProjectionMode(ProjectionMode mode) {
    render::application::GetRenderGateway().SetProjectionMode(mode);
}

void WorkbenchController::ResetView() {
    render::application::GetRenderGateway().ResetView();
}

void WorkbenchController::AlignCameraToActiveCellAxis(int axisIndex) {
    if (GetWorkbenchRuntime().AtomsTemplateFacade().IsBZPlotMode()) {
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
