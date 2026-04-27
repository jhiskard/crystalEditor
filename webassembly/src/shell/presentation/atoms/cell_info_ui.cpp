#include "cell_info_ui.h"
#include "../../../workspace/runtime/legacy_atoms_runtime.h"
#include "../../../structure/domain/atoms/cell_manager.h"   // cellInfo, calculateInverseMatrix, cartesianToFractional
#include "../../../config/log_config.h"
#include <cstdio>  // snprintf


namespace atoms {
namespace ui {

CellInfoUI::CellInfoUI(WorkspaceRuntimeModel* parent)
    : m_parent(parent) {
    SPDLOG_DEBUG("CellInfoUI initialized");
}

void CellInfoUI::applyCellChangesOnEditEnd() {
    if (!m_parent) {
        SPDLOG_ERROR("CellInfoUI::applyCellChangesOnEditEnd called with null parent");
        return;
    }

    // ���� �� ���� ������ ������ ����(WorkspaceRuntimeModel)���� ����
    m_parent->ApplyCellChangesFromEditor();
}

void CellInfoUI::render() {
    if (!m_parent) {
        ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f),
                           "CellInfoUI: WorkspaceRuntimeModel is not available.");
        return;
    }

    ImGuiTableFlags tableFlags =
        ImGuiTableFlags_SizingFixedFit | 
        ImGuiTableFlags_Borders |
        ImGuiTableFlags_RowBg;

    ImGuiTableFlags columnFlags = ImGuiTableColumnFlags_WidthStretch;
    
    // Edit mode ���
    ImGui::Checkbox("Edit mode##cellEdit", &m_editMode);

    // Edit mode ����( true �� false ) �� �� ���� ����
    if (m_prevEditMode && !m_editMode) {
        applyCellChangesOnEditEnd();
    }
    m_prevEditMode = m_editMode;
    
    // Cell matrix ���̺� ǥ��
    if (ImGui::BeginTable("CellMatrix", 4, tableFlags)) {
        // ��� ����
        ImGui::TableSetupColumn("",  columnFlags, 50.0f);
        ImGui::TableSetupColumn("x", columnFlags, 100.0f);
        ImGui::TableSetupColumn("y", columnFlags, 100.0f);
        ImGui::TableSetupColumn("z", columnFlags, 100.0f);
        ImGui::TableHeadersRow();
        
        // �� ��
        const char* rowLabels[] = { "v1", "v2", "v3" };
        
        for (int row = 0; row < 3; row++) {
            ImGui::TableNextRow();
            
            // ù ��° ��: �� ��
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", rowLabels[row]);
            
            // ������ ��: ��Ʈ���� ����
            for (int col = 0; col < 3; col++) {
                ImGui::TableSetColumnIndex(col + 1);

                if (m_editMode) {
                    char id[32];
                    std::snprintf(id, sizeof(id), "##cell_%d_%d", row, col);
                    if (ImGui::InputFloat(id, &cellInfo.matrix[row][col], 0.0f, 0.0f, "%.6f")) {
                        SPDLOG_DEBUG("Cell matrix[{}][{}] changed to: {:.6f}",
                                     row, col, cellInfo.matrix[row][col]);
                        cellInfo.modified = true;
                    }
                } else {
                    ImGui::Text("%.6f", cellInfo.matrix[row][col]);
                }
            }
        }
        
        ImGui::EndTable();
    }
    
    // Edit mode �ȳ� �޽���
    if (m_editMode) {
        ImGui::Separator();
        ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.0f, 1.0f), "[Edit Mode Active]");
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 1.0f, 1.0f),
                           "Uncheck 'Edit mode' to apply cell changes");
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f),
                           "? Changes will be batched for optimal performance");
    }
}

} // namespace ui
} // namespace atoms







