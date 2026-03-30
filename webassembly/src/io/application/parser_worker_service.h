#pragma once

#include "../../atoms/infrastructure/chgcar_parser.h"
#include "../../atoms/infrastructure/file_io_manager.h"

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
    void ProcessStructureFile(const std::string& fileName, ParserResultCallback callback) const;
    void ProcessXsfFile(const std::string& fileName, ParserResultCallback callback) const;
    void ProcessXsfGridFile(const std::string& fileName, ParserResultCallback callback) const;
    void ProcessChgcarFile(const std::string& fileName, ParserResultCallback callback) const;

private:
    static bool ContainsDatagrid3d(const std::string& filePath);
    static void DispatchToMainThread(ParserResultCallback callback, ParserWorkerResult* result);
};

} // namespace io::application
