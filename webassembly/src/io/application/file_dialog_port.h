#pragma once

namespace io::application {

/**
 * @brief File dialog port for browser-backed file picker interactions.
 */
class FileDialogPort {
public:
    virtual ~FileDialogPort() = default;

    virtual void OpenMeshFileBrowser(bool useMainThread) = 0;
    virtual void OpenMeshFileStreamingBrowser(bool useMainThread) = 0;
    virtual void OpenStructureFileBrowser() = 0;
    virtual void OpenXsfFileBrowser() = 0;
    virtual void OpenXsfGridFileBrowser() = 0;
    virtual void OpenChgcarFileBrowser() = 0;
};

} // namespace io::application

