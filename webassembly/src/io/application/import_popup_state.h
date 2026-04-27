#pragma once

#include <string>

namespace io::application {

/**
 * @brief Snapshot of import popup UI state exposed by FileLoader.
 */
struct ImportPopupState {
    bool showStructureReplacePopup = false;
    bool replaceSceneOnNextStructureImport = false;
    std::string deferredStructureFileName;

    bool showStructureImportErrorPopup = false;
    std::string structureImportErrorTitle;
    std::string structureImportErrorMessage;

    bool showXsfGridCellWarningPopup = false;
    std::string xsfGridCellWarningText;
};

} // namespace io::application

