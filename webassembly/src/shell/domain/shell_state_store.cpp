#include "shell_state_store.h"

namespace shell {
namespace domain {

ShellStateStore& ShellStateStore::Shared() {
    static ShellStateStore store;
    return store;
}

void ShellStateStore::Reset() {
    m_State = ShellUiState {};
}

ShellStateStore& GetShellStateStore() {
    return ShellStateStore::Shared();
}

} // namespace domain
} // namespace shell

