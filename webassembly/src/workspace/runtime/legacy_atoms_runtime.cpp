#include "legacy_atoms_runtime.h"

namespace workspace {
namespace legacy {

WorkspaceRuntimeModel& WorkspaceRuntimeModelRef() {
    static WorkspaceRuntimeModel runtime;
    return runtime;
}

const WorkspaceRuntimeModel& WorkspaceRuntimeModelRefConst() {
    return WorkspaceRuntimeModelRef();
}

} // namespace legacy
} // namespace workspace
