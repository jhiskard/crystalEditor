#pragma once

#include "file_io_manager.h"

#include <functional>
#include <string>

namespace io::infrastructure {

class XsfParser {
public:
    using ParseResult = atoms::infrastructure::FileIOManager::ParseResult;
    using Grid3DParseResult = atoms::infrastructure::FileIOManager::Grid3DParseResult;
    using ProgressCallback = std::function<void(float)>;

    static ParseResult parseStructureFile(
        const std::string& filePath,
        const ProgressCallback& progressCallback = {});
    static Grid3DParseResult parseGridFile(
        const std::string& filePath,
        const ProgressCallback& progressCallback = {});
};

} // namespace io::infrastructure
