#include "workspace_store.h"

namespace workspace {
namespace domain {

WorkspaceStore& WorkspaceStore::Shared() {
    static WorkspaceStore store;
    return store;
}

void WorkspaceStore::Reset() {
    m_CurrentStructureId = -1;
    m_SelectedMeshId = -1;
    m_ActiveDensityStructureId = -1;
    m_ActiveDensityGridMeshId = -1;
}

WorkspaceStore& GetWorkspaceStore() {
    return WorkspaceStore::Shared();
}

} // namespace domain
} // namespace workspace


