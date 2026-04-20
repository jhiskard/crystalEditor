target_sources(wb_io PRIVATE
  webassembly/src/io/application/import_entry_service.cpp
  webassembly/src/io/application/import_apply_service.cpp
  webassembly/src/io/application/import_orchestrator.cpp
  webassembly/src/io/application/import_workflow_service.cpp
  webassembly/src/io/application/parser_worker_service.cpp
  webassembly/src/io/infrastructure/chgcar_parser.cpp
  webassembly/src/io/infrastructure/file_io_manager.cpp
  webassembly/src/io/infrastructure/legacy/legacy_import_runtime_port.cpp
  webassembly/src/io/infrastructure/unv/unv_mesh_reader.cpp
  webassembly/src/io/infrastructure/xsf_parser.cpp
)
