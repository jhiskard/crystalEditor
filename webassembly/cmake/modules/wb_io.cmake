target_sources(wb_io PRIVATE
  webassembly/src/file_loader.cpp
  webassembly/src/unv_reader.cpp
  webassembly/src/io/application/import_apply_service.cpp
  webassembly/src/io/application/import_orchestrator.cpp
  webassembly/src/io/application/parser_worker_service.cpp
  webassembly/src/io/platform/browser_file_picker.cpp
  webassembly/src/atoms/infrastructure/chgcar_parser.cpp
  webassembly/src/atoms/infrastructure/file_io_manager.cpp
)
