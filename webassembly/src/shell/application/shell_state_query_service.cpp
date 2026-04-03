#include "shell_state_query_service.h"

namespace {

bool isWindowVisible(const shell::domain::ShellUiState& state, shell::domain::ShellWindowId windowId) {
    using shell::domain::ShellWindowId;

    switch (windowId) {
    case ShellWindowId::Viewer:
        return state.showVtkViewer;
    case ShellWindowId::ModelTree:
        return state.showModelTree;
    case ShellWindowId::TestWindow:
        return state.showTestWindow;
    case ShellWindowId::MeshDetail:
        return state.showMeshDetail;
    case ShellWindowId::PeriodicTable:
        return state.showPeriodicTableWindow;
    case ShellWindowId::CrystalTemplates:
        return state.showCrystalTemplatesWindow;
    case ShellWindowId::BrillouinZonePlot:
        return state.showBrillouinZonePlotWindow;
    case ShellWindowId::CreatedAtoms:
        return state.showCreatedAtomsWindow;
    case ShellWindowId::BondsManagement:
        return state.showBondsManagementWindow;
    case ShellWindowId::CellInformation:
        return state.showCellInformationWindow;
    case ShellWindowId::ChargeDensityViewer:
        return state.showChargeDensityViewerWindow;
    case ShellWindowId::SliceViewer:
        return state.showSliceViewerWindow;
    }

    return false;
}

} // namespace

namespace shell {
namespace application {

ShellStateQueryService& ShellStateQueryService::Instance() {
    static ShellStateQueryService service;
    return service;
}

const domain::ShellUiState& ShellStateQueryService::State() const {
    return domain::GetShellStateStore().State();
}

bool ShellStateQueryService::IsWindowVisible(domain::ShellWindowId windowId) const {
    return isWindowVisible(State(), windowId);
}

ShellStateQueryService& GetShellStateQueryService() {
    return ShellStateQueryService::Instance();
}

} // namespace application
} // namespace shell
