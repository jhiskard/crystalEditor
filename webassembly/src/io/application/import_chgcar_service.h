/**
 * @file import_chgcar_service.h
 * @brief CHGCAR parsing/apply bridge service for import workflow.
 */
#pragma once

#include "../infrastructure/chgcar_parser.h"

#include <functional>
#include <string>

namespace io::application {

/**
 * @brief Application service that handles CHGCAR parse/apply compatibility bridge.
 * @details Temporary compatibility path: apply flow still delegates to
 *          legacy runtime-backed implementation during W5.6 transition.
 */
class ImportChgcarService {
public:
    using ProgressCallback = std::function<void(float)>;

    atoms::infrastructure::ChgcarParser::ParseResult ParseChgcarFile(
        const std::string& filePath,
        const ProgressCallback& progressCallback) const;

    bool LoadChgcarParsedData(
        const atoms::infrastructure::ChgcarParser::ParseResult& parseResult) const;
};

} // namespace io::application
