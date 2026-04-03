#include "shell_state_store.h"

namespace shell {
namespace domain {

ShellStateStore& ShellStateStore::Instance() {
    static ShellStateStore store;
    return store;
}

void ShellStateStore::Reset() {
    m_State = ShellUiState {};
}

ShellStateStore& GetShellStateStore() {
    return ShellStateStore::Instance();
}

} // namespace domain
} // namespace shell
