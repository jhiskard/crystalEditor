#pragma once

#include "parser_worker_service.h"

// Standard library
#include <cstdint>
#include <string>

namespace io::application {

struct ImportApplyResult {
    bool success = false;
    int32_t importedStructureId = -1;
    std::string errorMessage;
    bool showXsfGridCellWarning = false;
    std::string xsfGridCellWarningText;
};

class ImportApplyService {
public:
    ImportApplyResult Apply(ParserWorkerResult& parsedResult) const;

private:
    ImportApplyResult ApplyXsf(ParserWorkerResult& parsedResult) const;
    ImportApplyResult ApplyXsfGrid(ParserWorkerResult& parsedResult) const;
    ImportApplyResult ApplyChgcar(ParserWorkerResult& parsedResult) const;
    bool ApplyXsfGridAtomsPayload(
        atoms::infrastructure::FileIOManager::Grid3DParseResult& parsedResult,
        int32_t structureId,
        const std::string& structureName,
        bool renderCell) const;
};

} // namespace io::application
