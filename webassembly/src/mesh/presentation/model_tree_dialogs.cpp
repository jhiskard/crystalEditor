#include "../../mesh/presentation/model_tree_panel.h"
#include "../../atoms/atoms_template.h"
#include "../application/mesh_command_service.h"
#include "../application/mesh_query_service.h"

#include <imgui.h>

void ModelTree::renderDeleteConfirmPopup() {
    if (m_ShowDeleteConfirmPopup) {
        ImGui::OpenPopup("Delete Confirmation");
    }
    
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    
    if (ImGui::BeginPopupModal("Delete Confirmation", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        std::string meshName = "Unknown";
        bool isXsfStructure = false;
        if (m_PendingDeleteMeshId != -1) {
            const Mesh* mesh = mesh::application::GetMeshQueryService().FindMeshById(m_PendingDeleteMeshId);
            if (mesh) {
                meshName = mesh->GetName();
                isXsfStructure = mesh->IsXsfStructure();
            }
        }
        
        ImGui::Text("%s will be removed.", meshName.c_str());
        ImGui::Text("Are you sure?");
        ImGui::Separator();
        
        float buttonWidth = 120.0f;
        float spacing = ImGui::GetStyle().ItemSpacing.x;
        float totalWidth = buttonWidth * 2 + spacing;
        float startX = (ImGui::GetWindowSize().x - totalWidth) * 0.5f;
        
        ImGui::SetCursorPosX(startX);

        if (ImGui::Button("Yes", ImVec2(buttonWidth, 0))) {
            if (m_PendingDeleteMeshId != -1) {
                if (isXsfStructure) {
                    if (m_SelectedMeshId == m_PendingDeleteMeshId) {
                        m_SelectedMeshId = -1;
                    }
                    AtomsTemplate::Instance().RemoveStructure(m_PendingDeleteMeshId);
                    mesh::application::GetMeshCommandService().DeleteXsfStructure(m_PendingDeleteMeshId);
                } else {
                    if (m_SelectedMeshId == m_PendingDeleteMeshId) {
                        m_SelectedMeshId = -1;
                    }
                    mesh::application::GetMeshCommandService().DeleteMesh(m_PendingDeleteMeshId);
                }
                m_PendingDeleteMeshId = -1;
            }
            
            m_ShowDeleteConfirmPopup = false;
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::SameLine();
        
        if (ImGui::Button("No", ImVec2(buttonWidth, 0))) {
            m_PendingDeleteMeshId = -1;
            m_ShowDeleteConfirmPopup = false;
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }
}


void ModelTree::renderClearMeasurementsConfirmPopup() {
    if (m_ShowClearMeasurementsConfirmPopup) {
        ImGui::OpenPopup("Clear Measurements Confirmation");
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Clear Measurements Confirmation", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("All Measurement entries will be removed.");
        ImGui::Text("Are you sure?");
        ImGui::Separator();

        float buttonWidth = 120.0f;
        float spacing = ImGui::GetStyle().ItemSpacing.x;
        float totalWidth = buttonWidth * 2 + spacing;
        float startX = (ImGui::GetWindowSize().x - totalWidth) * 0.5f;

        ImGui::SetCursorPosX(startX);
        if (ImGui::Button("Yes", ImVec2(buttonWidth, 0))) {
            if (m_PendingClearMeasurementsStructureId != -1) {
                AtomsTemplate::Instance().RemoveMeasurementsByStructure(
                    m_PendingClearMeasurementsStructureId);
            }
            m_PendingClearMeasurementsStructureId = -1;
            m_ShowClearMeasurementsConfirmPopup = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if (ImGui::Button("No", ImVec2(buttonWidth, 0))) {
            m_PendingClearMeasurementsStructureId = -1;
            m_ShowClearMeasurementsConfirmPopup = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}



