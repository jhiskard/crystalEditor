#include "runtime_progress_port.h"

#include "../../shell/runtime/workbench_runtime.h"

namespace platform::worker {

void RuntimeProgressPort::ReportProgress(float progress) {
    GetWorkbenchRuntime().SetProgress(progress);
}

} // namespace platform::worker

