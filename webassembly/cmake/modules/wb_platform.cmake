target_sources(wb_platform PRIVATE
  webassembly/src/platform/browser/browser_file_dialog_adapter.cpp
  webassembly/src/platform/browser/browser_file_picker.cpp
  webassembly/src/platform/persistence/viewer_preferences_store.cpp
  webassembly/src/platform/worker/emscripten_worker_port.cpp
  webassembly/src/platform/worker/runtime_progress_port.cpp
  webassembly/src/io/platform/memfs_chunk_stream.cpp
)
