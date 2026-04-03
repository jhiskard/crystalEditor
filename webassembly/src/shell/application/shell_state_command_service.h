/**
 * @file shell_state_command_service.h
 * @brief Write shell state command service.
 */
#pragma once

#include "../domain/shell_state_store.h"

namespace shell {
namespace application {

/**
 * @brief Write facade for shell UI state mutations.
 * @details Controller/menu actions should mutate shell state through this
 *          service so ownership remains centralized in the shell store.
 */
class ShellStateCommandService {
public:
    /**
     * @brief Returns singleton command service instance.
     */
    static ShellStateCommandService& Instance();

    /**
     * @brief Resets shell state to defaults.
     */
    void Reset();

    /**
     * @brief Returns mutable shell state snapshot.
     */
    domain::ShellUiState& MutableState();

    /**
     * @brief Sets visibility for a shell-managed window.
     */
    void SetWindowVisible(domain::ShellWindowId windowId, bool visible);

    /**
     * @brief Sets full dock-space mode flag.
     */
    void SetFullDockSpace(bool enabled);

    /**
     * @brief Requests a layout preset transition.
     */
    void RequestLayoutPreset(domain::ShellLayoutPreset preset);

    /**
     * @brief Requests focusing a target window for the next `passes` frames.
     */
    void RequestFocus(domain::ShellFocusTarget target, int passes = 2);

private:
    ShellStateCommandService() = default;
};

/**
 * @brief Convenience accessor for shell command service.
 */
ShellStateCommandService& GetShellStateCommandService();

} // namespace application
} // namespace shell
