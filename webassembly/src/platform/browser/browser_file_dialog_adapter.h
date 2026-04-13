#pragma once

#include "../../io/application/file_dialog_port.h"

namespace platform::browser {

/**
 * @brief Browser file dialog adapter implementing IO file dialog port.
 */
class BrowserFileDialogAdapter final : public io::application::FileDialogPort {
public:
    void OpenMeshFileBrowser(bool useMainThread) override;
    void OpenMeshFileStreamingBrowser(bool useMainThread) override;
    void OpenStructureFileBrowser() override;
    void OpenXsfFileBrowser() override;
    void OpenXsfGridFileBrowser() override;
    void OpenChgcarFileBrowser() override;
};

} // namespace platform::browser

