#pragma once

#include "../../io/application/progress_port.h"

namespace platform::worker {

/**
 * @brief Runtime-backed progress adapter for IO pipelines.
 */
class RuntimeProgressPort final : public io::application::ProgressPort {
public:
    void ReportProgress(float progress) override;
};

} // namespace platform::worker

