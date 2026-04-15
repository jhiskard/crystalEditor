#include "../legacy/atoms_template_facade.h"

#include "../ui/atom_editor_ui.h"
#include "../ui/bond_ui.h"
#include "../ui/cell_info_ui.h"

// ImGui
#include <imgui.h>

void AtomsTemplate::RenderCreatedAtomsWindow(bool* openWindow) {
    if (openWindow != nullptr && !*openWindow) {
        return;
    }

    if (m_PendingEditorSection == PendingEditorSection::Atoms) {
        m_PendingEditorSection = PendingEditorSection::None;
    }

    ImGui::SetNextWindowSize(ImVec2(450, 500), ImGuiCond_FirstUseEver);
    applyForcedWindowLayout(m_EditorWindowLayoutRequest);
    if (ImGui::Begin("Created Atoms", openWindow)) {
        if (m_atomEditorUI) {
            m_atomEditorUI->render();
        }
    }
    ImGui::End();
}

void AtomsTemplate::RenderBondsManagementWindow(bool* openWindow) {
    if (openWindow != nullptr && !*openWindow) {
        return;
    }

    if (m_PendingEditorSection == PendingEditorSection::Bonds) {
        m_PendingEditorSection = PendingEditorSection::None;
    }

    ImGui::SetNextWindowSize(ImVec2(450, 500), ImGuiCond_FirstUseEver);
    applyForcedWindowLayout(m_EditorWindowLayoutRequest);
    if (ImGui::Begin("Bonds Management", openWindow)) {
        if (m_bondUI) {
            m_bondUI->render();
        }
    }
    ImGui::End();
}

void AtomsTemplate::RenderCellInformationWindow(bool* openWindow) {
    if (openWindow != nullptr && !*openWindow) {
        return;
    }

    if (m_PendingEditorSection == PendingEditorSection::Cell) {
        m_PendingEditorSection = PendingEditorSection::None;
    }

    ImGui::SetNextWindowSize(ImVec2(450, 500), ImGuiCond_FirstUseEver);
    applyForcedWindowLayout(m_EditorWindowLayoutRequest);
    if (ImGui::Begin("Cell Information", openWindow)) {
        if (m_cellInfoUI) {
            m_cellInfoUI->render();
        }
    }
    ImGui::End();
}
