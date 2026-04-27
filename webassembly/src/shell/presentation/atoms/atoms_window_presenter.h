#pragma once

#include "../../domain/shell_state_store.h"

struct ImVec2;

namespace shell::presentation::atoms {

/**
 * @brief Thin-shim presenter for atom-related ImGui split windows.
 * @details W5.5 routes app-facing window rendering and panel requests through
 *          shell presentation boundary while legacy runtime still owns internals.
 */
class AtomsWindowPresenter {
public:
    void ApplyPendingPanelRequests(shell::domain::ShellUiState& shellState);

    void RequestForcedBuilderWindowLayout(const ImVec2& pos, const ImVec2& size, int frames = 1);
    void RequestForcedEditorWindowLayout(const ImVec2& pos, const ImVec2& size, int frames = 1);
    void RequestForcedAdvancedWindowLayout(const ImVec2& pos, const ImVec2& size, int frames = 1);

    void RenderPeriodicTableWindow(bool* openWindow = nullptr);
    void RenderCrystalTemplatesWindow(bool* openWindow = nullptr);
    void RenderCreatedAtomsWindow(bool* openWindow = nullptr);
    void RenderBondsManagementWindow(bool* openWindow = nullptr);
    void RenderCellInformationWindow(bool* openWindow = nullptr);
};

} // namespace shell::presentation::atoms
