#include "browser_file_picker.h"

// Emscripten
#include <emscripten/emscripten.h>

namespace io::platform {

void BrowserFilePicker::OpenMeshFileBrowser(bool useMainThread) {
    EM_ASM({
        let fileInput = document.createElement('input');
        fileInput.type = 'file';
        fileInput.multiple = false;
        fileInput.accept = '.vtk, .vtu, .unv';
        fileInput.onchange = () => {
            if (fileInput.files.length == 0) {
                return;
            }
            const file = fileInput.files[0];

            VtkModule.setProgressPopupText("Loading file", `File(${file.name}) is loading, please wait...`);
            VtkModule.showProgressPopup(true);

            const reader = new FileReader();
            reader.readAsArrayBuffer(file);
            reader.onload = () => {
                try {
                    const data = new Uint8Array(reader.result);
                    VtkModule.FS.createDataFile('/', file.name, data, true, false, true);

                    const useMainThread = $0;
                    if (useMainThread === 1) {
                        VtkModule.loadArrayBuffer(file.name, true);
                        VtkModule.showProgressPopup(false);
                    }
                    else if (useMainThread === 0) {
                        VtkModule.processFileInBackground(file.name, true);
                    }
                }
                catch (e) {
                    console.error("File loading error", e);
                }
            };
            reader.onerror = (e) => {
                console.error("File reading error", e);
            };
        };
        fileInput.click();
    }, useMainThread);
}

void BrowserFilePicker::OpenMeshFileStreamingBrowser(bool useMainThread) {
    EM_ASM({
        (async () => {
            try {
                const [fileHandle] = await window.showOpenFilePicker({
                    types: [
                      {
                        description: "Mesh files",
                        accept: {
                          "application/octet-stream": [".vtk", ".vtu", ".unv"],
                          "text/plain": [".vtk", ".vtu", ".unv"]
                        }
                      }
                    ],
                    excludeAcceptAllOption: true,
                    multiple: false
                });
                const file = await fileHandle.getFile();
                const stream = file.stream();
                const reader = stream.getReader();

                VtkModule.setProgressPopupText("Loading file", `File(${file.name}) is loading, please wait...`);
                VtkModule.showProgressPopup(true);

                let offset = 0;
                while (true) {
                    const { done, value } = await reader.read();
                    if (done) {
                        break;
                    }

                    const buffer = VtkModule._malloc(value.length);
                    VtkModule.HEAPU8.set(value, buffer);
                    VtkModule.writeChunk(file.name, offset, buffer, value.length);
                    VtkModule._free(buffer);

                    offset += value.length;
                    console.log(`Read ${offset} bytes from file: ${file.name} - ${(offset/file.size * 100).toFixed(2)}%`);
                }

                const useMainThread = $0;
                if (useMainThread === 1) {
                    VtkModule.loadArrayBuffer(file.name, true);
                    VtkModule.showProgressPopup(false);
                }
                else if (useMainThread === 0) {
                    VtkModule.processFileInBackground(file.name, true);
                }
            } catch (e) {
                console.error("File reading error:", e);
            }
        })();
    }, useMainThread);
}

void BrowserFilePicker::OpenStructureFileBrowser() {
    EM_ASM({
        let fileInput = document.createElement('input');
        fileInput.type = 'file';
        fileInput.multiple = false;
        fileInput.accept = '.xsf,.vasp,CHGCAR*,*';
        fileInput.onchange = () => {
            if (fileInput.files.length == 0) {
                return;
            }
            const file = fileInput.files[0];

            VtkModule.setProgressPopupText("Loading structure file", `File(${file.name}) is loading, please wait...`);
            VtkModule.showProgressPopup(true);

            const reader = new FileReader();
            reader.readAsArrayBuffer(file);
            reader.onload = () => {
                try {
                    const data = new Uint8Array(reader.result);
                    VtkModule.FS.createDataFile('/', file.name, data, true, false, true);
                    VtkModule.handleStructureFile(file.name);
                }
                catch (e) {
                    console.error("Structure file loading error", e);
                    VtkModule.showProgressPopup(false);
                }
            };
            reader.onerror = (e) => {
                console.error("File reading error", e);
                VtkModule.showProgressPopup(false);
            };
        };
        fileInput.click();
    });
}

void BrowserFilePicker::OpenXsfFileBrowser() {
    EM_ASM({
        let fileInput = document.createElement('input');
        fileInput.type = 'file';
        fileInput.multiple = false;
        fileInput.accept = '.xsf';
        fileInput.onchange = () => {
            if (fileInput.files.length == 0) {
                return;
            }
            const file = fileInput.files[0];

            VtkModule.setProgressPopupText("Loading XSF file", `File(${file.name}) is loading, please wait...`);
            VtkModule.showProgressPopup(true);

            const reader = new FileReader();
            reader.readAsArrayBuffer(file);
            reader.onload = () => {
                try {
                    const data = new Uint8Array(reader.result);
                    VtkModule.FS.createDataFile('/', file.name, data, true, false, true);
                    VtkModule.loadArrayBuffer(file.name, true);
                }
                catch (e) {
                    console.error("XSF file loading error", e);
                    VtkModule.showProgressPopup(false);
                }
            };
            reader.onerror = (e) => {
                console.error("File reading error", e);
                VtkModule.showProgressPopup(false);
            };
        };
        fileInput.click();
    });
}

void BrowserFilePicker::OpenXsfGridFileBrowser() {
    EM_ASM({
        let fileInput = document.createElement('input');
        fileInput.type = 'file';
        fileInput.multiple = false;
        fileInput.accept = '.xsf';
        fileInput.onchange = () => {
            if (fileInput.files.length == 0) {
                return;
            }
            const file = fileInput.files[0];

            VtkModule.setProgressPopupText("Loading XSF grid file", `File(${file.name}) is loading, please wait...`);
            VtkModule.showProgressPopup(true);

            const reader = new FileReader();
            reader.readAsArrayBuffer(file);
            reader.onload = () => {
                try {
                    const data = new Uint8Array(reader.result);
                    VtkModule.FS.createDataFile('/', file.name, data, true, false, true);
                    VtkModule.handleXSFGridFile(file.name);
                }
                catch (e) {
                    console.error("XSF grid file loading error", e);
                    VtkModule.showProgressPopup(false);
                }
            };
            reader.onerror = (e) => {
                console.error("File reading error", e);
                VtkModule.showProgressPopup(false);
            };
        };
        fileInput.click();
    });
}

void BrowserFilePicker::OpenChgcarFileBrowser() {
    EM_ASM({
        let fileInput = document.createElement('input');
        fileInput.type = 'file';
        fileInput.multiple = false;
        fileInput.accept = '.vasp,CHGCAR*,*';
        fileInput.onchange = () => {
            if (fileInput.files.length == 0) {
                return;
            }
            const file = fileInput.files[0];

            VtkModule.setProgressPopupText("Loading CHGCAR file", `File(${file.name}) is loading, please wait...`);
            VtkModule.showProgressPopup(true);

            const reader = new FileReader();
            reader.readAsArrayBuffer(file);
            reader.onload = () => {
                try {
                    const data = new Uint8Array(reader.result);
                    VtkModule.FS.createDataFile('/', file.name, data, true, false, true);
                    VtkModule.loadChgcarFile(file.name);
                }
                catch (e) {
                    console.error("CHGCAR file loading error", e);
                    VtkModule.showProgressPopup(false);
                }
            };
            reader.onerror = (e) => {
                console.error("File reading error", e);
                VtkModule.showProgressPopup(false);
            };
        };
        fileInput.click();
    });
}

} // namespace io::platform
