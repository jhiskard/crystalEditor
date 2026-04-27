#include "import_chgcar_service.h"

#include "../../workspace/runtime/legacy_atoms_runtime.h"

namespace io::application {

atoms::infrastructure::ChgcarParser::ParseResult ImportChgcarService::ParseChgcarFile(
    const std::string& filePath,
    const ProgressCallback& progressCallback) const {
    return atoms::infrastructure::ChgcarParser::parse(filePath, progressCallback);
}

bool ImportChgcarService::LoadChgcarParsedData(
    const atoms::infrastructure::ChgcarParser::ParseResult& parseResult) const {
    return workspace::legacy::WorkspaceRuntimeModelRef().LoadChgcarParsedData(parseResult);
}

} // namespace io::application
