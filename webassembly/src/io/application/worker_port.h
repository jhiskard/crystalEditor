#pragma once

namespace io::application {

using MainThreadCallback = void (*)(int, const void*);

/**
 * @brief Worker port for crossing between worker threads and main runtime thread.
 */
class WorkerPort {
public:
    virtual ~WorkerPort() = default;

    virtual void DispatchToMainThreadAsync(MainThreadCallback callback, const void* payload) = 0;
    virtual void DispatchToMainThreadSync(MainThreadCallback callback, const void* payload) = 0;
};

} // namespace io::application

