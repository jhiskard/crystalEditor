#include "../../app.h"

#include "shell/application/shell_state_command_service.h"
#include "shell/presentation/window_layout.h"
#include "shell/runtime/workbench_runtime.h"

namespace shell::presentation {

ResetWindowLayout BuildResetWindowLayout(const ImGuiViewport* viewport) {
    const ImVec2 workPos = viewport->WorkPos;
    const ImVec2 workSize = viewport->WorkSize;

    const ImVec2 viewerSize(workSize.x * 0.5f, workSize.y * 0.5f);
    const ImVec2 viewerPos(
        workPos.x + (workSize.x - viewerSize.x) * 0.5f,
        workPos.y + (workSize.y - viewerSize.y) * 0.5f);

    const ImVec2 panelSize(workSize.x * 0.3f, workSize.y * 0.5f);
    const float inset5x = workSize.x * 0.05f;
    const float inset5y = workSize.y * 0.05f;
    const float inset10x = workSize.x * 0.10f;
    const float inset10y = workSize.y * 0.10f;

    ResetWindowLayout layout {};
    layout.viewerPos = viewerPos;
    layout.viewerSize = viewerSize;
    layout.panelSize = panelSize;
    layout.modelTreePos = ImVec2(workPos.x + workSize.x - panelSize.x - inset5x, workPos.y + inset5y);
    layout.advancedViewPos = ImVec2(workPos.x + workSize.x - panelSize.x - inset10x, workPos.y + inset10y);
    layout.crystalBuilderPos = ImVec2(workPos.x + inset5x, workPos.y + inset5y);
    layout.crystalEditorPos = ImVec2(workPos.x + inset10x, workPos.y + inset10y);
    return layout;
}

}  // namespace shell::presentation

void App::RequestLayout1() {
    shell::domain::ShellUiState& shellState = GetWorkbenchRuntime().ShellStateCommand().MutableState();
    shellState.pendingLayoutPreset = shell::domain::ShellLayoutPreset::DefaultFloating;
    shellState.requestModelTreeFocus = false;
    shellState.pendingFocusTarget = shell::domain::ShellFocusTarget::None;
    shellState.pendingFocusPassesRemaining = 0;
}
