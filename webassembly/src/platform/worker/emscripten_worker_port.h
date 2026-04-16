#pragma once

#include "../../io/application/worker_port.h"

namespace platform::worker {

/**
 * @brief Emscripten main-thread dispatcher adapter.
 */
class EmscriptenWorkerPort final : public io::application::WorkerPort {
public:
    void DispatchToMainThreadAsync(io::application::MainThreadCallback callback, const void* payload) override;
    void DispatchToMainThreadSync(io::application::MainThreadCallback callback, const void* payload) override;
};

} // namespace platform::worker

