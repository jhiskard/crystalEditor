#include "browser_file_dialog_adapter.h"

#include "browser_file_picker.h"

namespace platform::browser {

void BrowserFileDialogAdapter::OpenMeshFileBrowser(bool useMainThread) {
    BrowserFilePicker::OpenMeshFileBrowser(useMainThread);
}

void BrowserFileDialogAdapter::OpenMeshFileStreamingBrowser(bool useMainThread) {
    BrowserFilePicker::OpenMeshFileStreamingBrowser(useMainThread);
}

void BrowserFileDialogAdapter::OpenStructureFileBrowser() {
    BrowserFilePicker::OpenStructureFileBrowser();
}

void BrowserFileDialogAdapter::OpenXsfFileBrowser() {
    BrowserFilePicker::OpenXsfFileBrowser();
}

void BrowserFileDialogAdapter::OpenXsfGridFileBrowser() {
    BrowserFilePicker::OpenXsfGridFileBrowser();
}

void BrowserFileDialogAdapter::OpenChgcarFileBrowser() {
    BrowserFilePicker::OpenChgcarFileBrowser();
}

} // namespace platform::browser

