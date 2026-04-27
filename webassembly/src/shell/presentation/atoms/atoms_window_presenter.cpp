#include "atoms_window_presenter.h"

#include "../../../workspace/runtime/legacy_atoms_runtime.h"

namespace shell::presentation::atoms {

void AtomsWindowPresenter::ApplyPendingPanelRequests(shell::domain::ShellUiState& shellState) {
    WorkspaceRuntimeModel& atomsTemplate = workspace::legacy::WorkspaceRuntimeModelRef();

    if (shellState.hasPendingEditorRequest) {
        atomsTemplate.RequestEditorSection(shellState.pendingEditorRequest);
        shellState.hasPendingEditorRequest = false;
    }
    if (shellState.hasPendingBuilderRequest) {
        atomsTemplate.RequestBuilderSection(shellState.pendingBuilderRequest);
        shellState.hasPendingBuilderRequest = false;
    }
    if (shellState.hasPendingDataRequest) {
        atomsTemplate.RequestDataMenu(shellState.pendingDataRequest);
        shellState.hasPendingDataRequest = false;
    }
}

void AtomsWindowPresenter::RequestForcedBuilderWindowLayout(
    const ImVec2& pos,
    const ImVec2& size,
    int frames) {
    workspace::legacy::WorkspaceRuntimeModelRef().RequestForcedBuilderWindowLayout(pos, size, frames);
}

void AtomsWindowPresenter::RequestForcedEditorWindowLayout(
    const ImVec2& pos,
    const ImVec2& size,
    int frames) {
    workspace::legacy::WorkspaceRuntimeModelRef().RequestForcedEditorWindowLayout(pos, size, frames);
}

void AtomsWindowPresenter::RequestForcedAdvancedWindowLayout(
    const ImVec2& pos,
    const ImVec2& size,
    int frames) {
    workspace::legacy::WorkspaceRuntimeModelRef().RequestForcedAdvancedWindowLayout(pos, size, frames);
}

void AtomsWindowPresenter::RenderPeriodicTableWindow(bool* openWindow) {
    workspace::legacy::WorkspaceRuntimeModelRef().RenderPeriodicTableWindow(openWindow);
}

void AtomsWindowPresenter::RenderCrystalTemplatesWindow(bool* openWindow) {
    workspace::legacy::WorkspaceRuntimeModelRef().RenderCrystalTemplatesWindow(openWindow);
}

void AtomsWindowPresenter::RenderCreatedAtomsWindow(bool* openWindow) {
    workspace::legacy::WorkspaceRuntimeModelRef().RenderCreatedAtomsWindow(openWindow);
}

void AtomsWindowPresenter::RenderBondsManagementWindow(bool* openWindow) {
    workspace::legacy::WorkspaceRuntimeModelRef().RenderBondsManagementWindow(openWindow);
}

void AtomsWindowPresenter::RenderCellInformationWindow(bool* openWindow) {
    workspace::legacy::WorkspaceRuntimeModelRef().RenderCellInformationWindow(openWindow);
}

} // namespace shell::presentation::atoms
