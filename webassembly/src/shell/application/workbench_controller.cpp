#include "workbench_controller.h"

#include "shell_state_command_service.h"
#include "../../atoms/atoms_template.h"
#include "../../file_loader.h"
#include "../../mesh_manager.h"
#include "../../render/application/render_gateway.h"

#include <algorithm>

namespace {

void requestFocus(shell::domain::ShellFocusTarget target, int passes = 2) {
    shell::application::GetShellStateCommandService().RequestFocus(target, passes);
}

shell::domain::ShellUiState& mutableShellState() {
    return shell::application::GetShellStateCommandService().MutableState();
}

measurement::application::MeasurementService& measurementService() {
    return AtomsTemplate::Instance().measurementService();
}

} // namespace

namespace shell {
namespace application {

WorkbenchController& WorkbenchController::Instance() {
    static WorkbenchController controller;
    return controller;
}

void WorkbenchController::RequestOpenStructureImport() {
    FileLoader::Instance().RequestOpenStructureImport();
}

void WorkbenchController::OpenEditorPanel(EditorPanelAction action) {
    domain::ShellUiState& state = mutableShellState();

    switch (action) {
    case EditorPanelAction::Atoms:
        state.showCreatedAtomsWindow = true;
        requestFocus(domain::ShellFocusTarget::CreatedAtoms);
        AtomsTemplate::Instance().RequestEditorSection(AtomsTemplate::EditorSectionRequest::Atoms);
        break;
    case EditorPanelAction::Bonds:
        state.showBondsManagementWindow = true;
        requestFocus(domain::ShellFocusTarget::BondsManagement);
        AtomsTemplate::Instance().RequestEditorSection(AtomsTemplate::EditorSectionRequest::Bonds);
        break;
    case EditorPanelAction::Cell:
        state.showCellInformationWindow = true;
        requestFocus(domain::ShellFocusTarget::CellInformation);
        AtomsTemplate::Instance().RequestEditorSection(AtomsTemplate::EditorSectionRequest::Cell);
        break;
    }
}

void WorkbenchController::OpenBuilderPanel(BuilderPanelAction action) {
    domain::ShellUiState& state = mutableShellState();

    switch (action) {
    case BuilderPanelAction::AddAtoms:
        state.showPeriodicTableWindow = true;
        requestFocus(domain::ShellFocusTarget::PeriodicTable);
        AtomsTemplate::Instance().RequestBuilderSection(AtomsTemplate::BuilderSectionRequest::AddAtoms);
        break;
    case BuilderPanelAction::BravaisLatticeTemplates:
        state.showCrystalTemplatesWindow = true;
        requestFocus(domain::ShellFocusTarget::CrystalTemplates);
        AtomsTemplate::Instance().RequestBuilderSection(
            AtomsTemplate::BuilderSectionRequest::BravaisLatticeTemplates);
        break;
    case BuilderPanelAction::BrillouinZone:
        state.showBrillouinZonePlotWindow = true;
        requestFocus(domain::ShellFocusTarget::BrillouinZonePlot);
        AtomsTemplate::Instance().RequestBuilderSection(AtomsTemplate::BuilderSectionRequest::BrillouinZone);
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
        AtomsTemplate::Instance().RequestDataMenu(AtomsTemplate::DataMenuRequest::Isosurface);
        break;
    case DataPanelAction::Surface:
        state.showChargeDensityViewerWindow = true;
        requestFocus(domain::ShellFocusTarget::ChargeDensityViewer);
        AtomsTemplate::Instance().RequestDataMenu(AtomsTemplate::DataMenuRequest::Surface);
        break;
    case DataPanelAction::Volumetric:
        state.showChargeDensityViewerWindow = true;
        requestFocus(domain::ShellFocusTarget::ChargeDensityViewer);
        AtomsTemplate::Instance().RequestDataMenu(AtomsTemplate::DataMenuRequest::Volumetric);
        break;
    case DataPanelAction::Plane:
        state.showSliceViewerWindow = true;
        requestFocus(domain::ShellFocusTarget::SliceViewer);
        AtomsTemplate::Instance().RequestDataMenu(AtomsTemplate::DataMenuRequest::Plane);
        break;
    }
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
    AtomsTemplate& atomsTemplate = AtomsTemplate::Instance();
    atomsTemplate.SetBoundaryAtomsEnabled(!atomsTemplate.isSurroundingsVisible());
}

bool WorkbenchController::IsBoundaryAtomsEnabled() const {
    return AtomsTemplate::Instance().isSurroundingsVisible();
}

void WorkbenchController::SetMeshDisplayMode(MeshDisplayMode mode) {
    MeshManager::Instance().SetAllDisplayMode(mode);
}

void WorkbenchController::SetProjectionMode(ProjectionMode mode) {
    render::application::GetRenderGateway().SetProjectionMode(mode);
}

void WorkbenchController::ResetView() {
    render::application::GetRenderGateway().ResetView();
}

void WorkbenchController::AlignCameraToActiveCellAxis(int axisIndex) {
    if (AtomsTemplate::Instance().IsBZPlotMode()) {
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
