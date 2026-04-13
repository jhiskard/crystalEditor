#pragma once

namespace io::application {

/**
 * @brief Progress reporting port used by IO parsing/reading pipelines.
 */
class ProgressPort {
public:
    virtual ~ProgressPort() = default;

    virtual void ReportProgress(float progress) = 0;
};

} // namespace io::application

