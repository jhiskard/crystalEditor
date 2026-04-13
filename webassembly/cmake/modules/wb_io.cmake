target_sources(wb_io PRIVATE
  webassembly/src/io/application/import_entry_service.cpp
  webassembly/src/io/infrastructure/unv/unv_mesh_reader.cpp
  webassembly/src/io/application/import_apply_service.cpp
  webassembly/src/io/application/import_workflow_service.cpp
  webassembly/src/io/application/import_orchestrator.cpp
  webassembly/src/io/application/parser_worker_service.cpp
  webassembly/src/io/infrastructure/xsf_parser.cpp
  webassembly/src/io/platform/memfs_chunk_stream.cpp
  webassembly/src/platform/browser/browser_file_dialog_adapter.cpp
  webassembly/src/platform/worker/emscripten_worker_port.cpp
  webassembly/src/platform/worker/runtime_progress_port.cpp
  webassembly/src/platform/browser/browser_file_picker.cpp
  webassembly/src/atoms/infrastructure/chgcar_parser.cpp
  webassembly/src/atoms/infrastructure/file_io_manager.cpp
)
