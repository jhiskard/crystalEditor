#include "import_popup_presenter.h"

#include "../../runtime/workbench_runtime.h"
#include "../../../io/application/import_entry_service.h"

// ImGui
#include <imgui.h>

// Standard library
#include <cfloat>
#include <cstdio>
#include <string>

namespace shell::presentation {

void ImportPopupPresenter::Render(FileLoader& fileLoader) {
    if (fileLoader.m_ShowStructureReplacePopup) {
        ImGui::OpenPopup("Clear Current Data and Import?");
        fileLoader.m_ShowStructureReplacePopup = false;
    }
    ImGui::SetNextWindowSizeConstraints(ImVec2(520.0f, 0.0f), ImVec2(FLT_MAX, FLT_MAX));
    if (ImGui::BeginPopupModal("Clear Current Data and Import?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextWrapped("Viewer is not empty.");
        ImGui::TextWrapped("Do you want to clear all current data and continue importing?");

        if (ImGui::Button("Yes")) {
            if (!fileLoader.m_DeferredStructureFileName.empty()) {
                fileLoader.m_ReplaceSceneOnNextStructureImport = true;
                GetWorkbenchRuntime().SetProgressPopupText(
                    "Loading structure file",
                    "File(" + fileLoader.m_DeferredStructureFileName + ") is loading, please wait...");
                GetWorkbenchRuntime().ShowProgressPopup(true);
                const std::string deferredFileName = fileLoader.m_DeferredStructureFileName;
                fileLoader.m_DeferredStructureFileName.clear();
                fileLoader.handleStructureFile(deferredFileName);
            } else {
                fileLoader.m_ReplaceSceneOnNextStructureImport = true;
                fileLoader.OpenStructureFileBrowser();
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("No")) {
            fileLoader.m_ReplaceSceneOnNextStructureImport = false;
            if (!fileLoader.m_DeferredStructureFileName.empty()) {
                const std::string memfsPath = "/" + fileLoader.m_DeferredStructureFileName;
                std::remove(memfsPath.c_str());
                fileLoader.m_DeferredStructureFileName.clear();
            }
            GetWorkbenchRuntime().ShowProgressPopup(false);
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (fileLoader.m_ShowStructureImportErrorPopup) {
        ImGui::OpenPopup("Import Structure Failed");
        fileLoader.m_ShowStructureImportErrorPopup = false;
    }
    ImGui::SetNextWindowSizeConstraints(ImVec2(540.0f, 0.0f), ImVec2(FLT_MAX, FLT_MAX));
    if (ImGui::BeginPopupModal("Import Structure Failed", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        if (!fileLoader.m_StructureImportErrorTitle.empty()) {
            ImGui::TextUnformatted(fileLoader.m_StructureImportErrorTitle.c_str());
            ImGui::Separator();
        }
        if (fileLoader.m_StructureImportErrorMessage.empty()) {
            ImGui::TextWrapped("The selected structure file could not be imported.");
        } else {
            ImGui::TextWrapped("%s", fileLoader.m_StructureImportErrorMessage.c_str());
        }
        if (ImGui::Button("OK")) {
            fileLoader.m_StructureImportErrorTitle.clear();
            fileLoader.m_StructureImportErrorMessage.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (fileLoader.m_ShowXsfGridCellWarningPopup) {
        ImGui::OpenPopup("XSF Cell Warning");
        fileLoader.m_ShowXsfGridCellWarningPopup = false;
    }
    if (ImGui::BeginPopupModal("XSF Cell Warning", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        if (fileLoader.m_XsfGridCellWarningText.empty()) {
            ImGui::TextWrapped("Cell vectors differ across DATAGRID_3D blocks.");
        } else {
            ImGui::TextWrapped("%s", fileLoader.m_XsfGridCellWarningText.c_str());
        }
        if (ImGui::Button("OK")) {
            fileLoader.m_XsfGridCellWarningText.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

} // namespace shell::presentation

