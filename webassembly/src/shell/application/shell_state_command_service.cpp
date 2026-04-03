#include "shell_state_command_service.h"

namespace {

void setWindowVisible(shell::domain::ShellUiState& state, shell::domain::ShellWindowId windowId, bool visible) {
    using shell::domain::ShellWindowId;

    switch (windowId) {
    case ShellWindowId::Viewer:
        state.showVtkViewer = visible;
        return;
    case ShellWindowId::ModelTree:
        state.showModelTree = visible;
        return;
    case ShellWindowId::TestWindow:
        state.showTestWindow = visible;
        return;
    case ShellWindowId::MeshDetail:
        state.showMeshDetail = visible;
        return;
    case ShellWindowId::PeriodicTable:
        state.showPeriodicTableWindow = visible;
        return;
    case ShellWindowId::CrystalTemplates:
        state.showCrystalTemplatesWindow = visible;
        return;
    case ShellWindowId::BrillouinZonePlot:
        state.showBrillouinZonePlotWindow = visible;
        return;
    case ShellWindowId::CreatedAtoms:
        state.showCreatedAtomsWindow = visible;
        return;
    case ShellWindowId::BondsManagement:
        state.showBondsManagementWindow = visible;
        return;
    case ShellWindowId::CellInformation:
        state.showCellInformationWindow = visible;
        return;
    case ShellWindowId::ChargeDensityViewer:
        state.showChargeDensityViewerWindow = visible;
        return;
    case ShellWindowId::SliceViewer:
        state.showSliceViewerWindow = visible;
        return;
    }
}

} // namespace

namespace shell {
namespace application {

ShellStateCommandService& ShellStateCommandService::Instance() {
    static ShellStateCommandService service;
    return service;
}

void ShellStateCommandService::Reset() {
    domain::GetShellStateStore().Reset();
}

domain::ShellUiState& ShellStateCommandService::MutableState() {
    return domain::GetShellStateStore().MutableState();
}

void ShellStateCommandService::SetWindowVisible(domain::ShellWindowId windowId, bool visible) {
    setWindowVisible(MutableState(), windowId, visible);
}

void ShellStateCommandService::SetFullDockSpace(bool enabled) {
    MutableState().fullDockSpace = enabled;
}

void ShellStateCommandService::RequestLayoutPreset(domain::ShellLayoutPreset preset) {
    MutableState().pendingLayoutPreset = preset;
}

void ShellStateCommandService::RequestFocus(domain::ShellFocusTarget target, int passes) {
    domain::ShellUiState& state = MutableState();
    state.pendingFocusTarget = target;
    state.pendingFocusPassesRemaining = passes;
}

ShellStateCommandService& GetShellStateCommandService() {
    return ShellStateCommandService::Instance();
}

} // namespace application
} // namespace shell
