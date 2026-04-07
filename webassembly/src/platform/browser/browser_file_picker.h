#pragma once

namespace platform::browser {

class BrowserFilePicker {
public:
    static void OpenMeshFileBrowser(bool useMainThread);
    static void OpenMeshFileStreamingBrowser(bool useMainThread);
    static void OpenStructureFileBrowser();
    static void OpenXsfFileBrowser();
    static void OpenXsfGridFileBrowser();
    static void OpenChgcarFileBrowser();
};

} // namespace platform::browser
