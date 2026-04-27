#include "../../../workspace/runtime/legacy_atoms_runtime.h"

#include "bravais_lattice_ui.h"
#include "bz_plot_ui.h"
#include "periodic_table_ui.h"

// ImGui
#include <imgui.h>

void WorkspaceRuntimeModel::RenderCrystalTemplatesWindow(bool* openWindow) {
    if (openWindow != nullptr && !*openWindow) {
        return;
    }

    if (m_PendingBuilderSection == PendingBuilderSection::BravaisLatticeTemplates) {
        m_PendingBuilderSection = PendingBuilderSection::None;
    }

    ImGui::SetNextWindowSize(ImVec2(500, 600), ImGuiCond_FirstUseEver);
    applyForcedWindowLayout(m_BuilderWindowLayoutRequest);
    if (ImGui::Begin("Crystal Templates", openWindow)) {
        if (m_bravaisLatticeUI) {
            m_bravaisLatticeUI->render();
        }
    }
    ImGui::End();
}

void WorkspaceRuntimeModel::RenderPeriodicTableWindow(bool* openWindow) {
    if (openWindow != nullptr && !*openWindow) {
        return;
    }

    if (m_PendingBuilderSection == PendingBuilderSection::AddAtoms) {
        m_PendingBuilderSection = PendingBuilderSection::None;
    }

    ImGui::SetNextWindowSize(ImVec2(500, 600), ImGuiCond_FirstUseEver);
    applyForcedWindowLayout(m_BuilderWindowLayoutRequest);
    if (ImGui::Begin("Periodic Table", openWindow)) {
        if (m_periodicTableUI) {
            m_periodicTableUI->render();
        }
    }
    ImGui::End();
}

void WorkspaceRuntimeModel::RenderBrillouinZonePlotWindow(bool* openWindow) {
    if (openWindow != nullptr && !*openWindow) {
        return;
    }

    if (m_PendingBuilderSection == PendingBuilderSection::BrillouinZone) {
        m_PendingBuilderSection = PendingBuilderSection::None;
    }

    ImGui::SetNextWindowSize(ImVec2(500, 600), ImGuiCond_FirstUseEver);
    applyForcedWindowLayout(m_BuilderWindowLayoutRequest);
    if (ImGui::Begin("Brillouin Zone Plot", openWindow)) {
        if (m_bzPlotUI) {
            m_bzPlotUI->render();
        }
    }
    ImGui::End();
}







