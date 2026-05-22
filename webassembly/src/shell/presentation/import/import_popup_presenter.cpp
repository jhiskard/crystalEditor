#include "import_popup_presenter.h"

#include "../../../io/application/import_entry_service.h"

// ImGui
#include <imgui.h>

// Standard library
#include <cfloat>

namespace shell::presentation {

void ImportPopupPresenter::Render(FileLoader& fileLoader) {
    const auto& popupState = fileLoader.getImportPopupState();

    if (popupState.showStructureReplacePopup) {
        ImGui::OpenPopup("Clear Current Data and Import?");
    }
    ImGui::SetNextWindowSizeConstraints(ImVec2(520.0f, 0.0f), ImVec2(FLT_MAX, FLT_MAX));
    if (ImGui::BeginPopupModal("Clear Current Data and Import?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextWrapped("Viewer is not empty.");
        ImGui::TextWrapped("Do you want to clear all current data and continue importing?");

        if (ImGui::Button("Yes")) {
            fileLoader.acknowledgeStructureReplacePopup(true);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("No")) {
            fileLoader.acknowledgeStructureReplacePopup(false);
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (popupState.showStructureImportErrorPopup) {
        ImGui::OpenPopup("Import Structure Failed");
    }
    ImGui::SetNextWindowSizeConstraints(ImVec2(540.0f, 0.0f), ImVec2(FLT_MAX, FLT_MAX));
    if (ImGui::BeginPopupModal("Import Structure Failed", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        if (!popupState.structureImportErrorTitle.empty()) {
            ImGui::TextUnformatted(popupState.structureImportErrorTitle.c_str());
            ImGui::Separator();
        }
        if (popupState.structureImportErrorMessage.empty()) {
            ImGui::TextWrapped("The selected structure file could not be imported.");
        } else {
            ImGui::TextWrapped("%s", popupState.structureImportErrorMessage.c_str());
        }
        if (ImGui::Button("OK")) {
            fileLoader.dismissStructureImportErrorPopup();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (popupState.showXsfGridCellWarningPopup) {
        ImGui::OpenPopup("XSF Cell Warning");
    }
    if (ImGui::BeginPopupModal("XSF Cell Warning", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        if (popupState.xsfGridCellWarningText.empty()) {
            ImGui::TextWrapped("Cell vectors differ across DATAGRID_3D blocks.");
        } else {
            ImGui::TextWrapped("%s", popupState.xsfGridCellWarningText.c_str());
        }
        if (ImGui::Button("OK")) {
            fileLoader.dismissXsfGridCellWarningPopup();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

} // namespace shell::presentation

