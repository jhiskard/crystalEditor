#include "emscripten_worker_port.h"

#include <emscripten/threading.h>

namespace platform::worker {

void EmscriptenWorkerPort::DispatchToMainThreadAsync(
    io::application::MainThreadCallback callback,
    const void* payload) {
    if (callback == nullptr) {
        return;
    }

    emscripten_async_run_in_main_runtime_thread(
        EM_FUNC_SIG_VIP,
        callback,
        0,
        payload);
}

void EmscriptenWorkerPort::DispatchToMainThreadSync(
    io::application::MainThreadCallback callback,
    const void* payload) {
    if (callback == nullptr) {
        return;
    }

    emscripten_sync_run_in_main_runtime_thread(
        EM_FUNC_SIG_VIP,
        callback,
        0,
        payload);
}

} // namespace platform::worker

