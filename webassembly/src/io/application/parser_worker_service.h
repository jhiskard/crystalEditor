#pragma once

#include "progress_port.h"
#include "worker_port.h"

#include "../infrastructure/chgcar_parser.h"
#include "../infrastructure/file_io_manager.h"

// Standard library
#include <string>

namespace io::application {

enum class ParsedImportKind {
    XsfStructure,
    XsfGrid,
    Chgcar
};

struct ParserWorkerResult {
    ParsedImportKind kind = ParsedImportKind::XsfStructure;
    std::string filePath;
    atoms::infrastructure::FileIOManager::ParseResult xsfResult;
    atoms::infrastructure::FileIOManager::Grid3DParseResult xsfGridResult;
    atoms::infrastructure::ChgcarParser::ParseResult chgcarResult;
};

using ParserResultCallback = void (*)(int, const void*);

class ParserWorkerService {
public:
    ParserWorkerService(WorkerPort& workerPort, ProgressPort& progressPort);

    void ProcessStructureFile(const std::string& fileName, ParserResultCallback callback) const;
    void ProcessXsfFile(const std::string& fileName, ParserResultCallback callback) const;
    void ProcessXsfGridFile(const std::string& fileName, ParserResultCallback callback) const;
    void ProcessChgcarFile(const std::string& fileName, ParserResultCallback callback) const;

private:
    WorkerPort& m_WorkerPort;
    ProgressPort& m_ProgressPort;

    static bool ContainsDatagrid3d(const std::string& filePath);
    void DispatchToMainThread(ParserResultCallback callback, ParserWorkerResult* result) const;
};

} // namespace io::application
