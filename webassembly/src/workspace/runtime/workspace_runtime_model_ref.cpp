#include "workspace_runtime_model_ref.h"

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
