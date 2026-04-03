/**
 * @file shell_state_query_service.h
 * @brief Read-only shell state query service.
 */
#pragma once

#include "../domain/shell_state_store.h"

namespace shell {
namespace application {

/**
 * @brief Read facade for shell UI state.
 * @details Presentation layers can use this service to inspect shell state
 *          without mutating the underlying store directly.
 */
class ShellStateQueryService {
public:
    /**
     * @brief Returns singleton query service instance.
     */
    static ShellStateQueryService& Instance();

    /**
     * @brief Returns immutable shell state snapshot.
     */
    const domain::ShellUiState& State() const;

    /**
     * @brief Returns visibility flag for a shell-managed window.
     */
    bool IsWindowVisible(domain::ShellWindowId windowId) const;

private:
    ShellStateQueryService() = default;
};

/**
 * @brief Convenience accessor for shell query service.
 */
ShellStateQueryService& GetShellStateQueryService();

} // namespace application
} // namespace shell
