#include "model_tree.h"
#include "app.h"
#include "font_manager.h"
#include "mesh_manager.h"
#include "lcrs_tree.h"
#include "mesh_detail.h"

// ImGui
#include <imgui.h>
#include "atoms/atoms_template.h"
#include "atoms/ui/charge_density_ui.h"
#include "atoms/application/structure_read_model.h"

int32_t ModelTree::s_DeleteMeshId = -1;
int32_t ModelTree::s_SelectedMeshId = -1;
int32_t ModelTree::s_PendingDeleteMeshId = -1;
bool ModelTree::s_ShowDeleteConfirmPopup = false;
int32_t ModelTree::s_PendingClearMeasurementsStructureId = -1;
bool ModelTree::s_ShowClearMeasurementsConfirmPopup = false;

ModelTree::ModelTree() {
}

ModelTree::~ModelTree() {
}

// 헬퍼 함수
void TextColoredCentered(const ImVec4& color, const char* text) {
    float columnWidth = ImGui::GetColumnWidth();
    float textWidth = ImGui::CalcTextSize(text).x;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (columnWidth - textWidth) * 0.5f);
    ImGui::TextColored(color, "%s", text);
}

void TextCentered(const char* text) {
    float columnWidth = ImGui::GetColumnWidth();
    float textWidth = ImGui::CalcTextSize(text).x;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (columnWidth - textWidth) * 0.5f);
    ImGui::Text("%s", text);
}

void TextCenteredInt(int value) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", value);
    float columnWidth = ImGui::GetColumnWidth();
    float textWidth = ImGui::CalcTextSize(buf).x;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (columnWidth - textWidth) * 0.5f);
    ImGui::Text("%s", buf);
}

void TextCenteredSizeT(size_t value) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%zu", value);
    float columnWidth = ImGui::GetColumnWidth();
    float textWidth = ImGui::CalcTextSize(buf).x;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (columnWidth - textWidth) * 0.5f);
    ImGui::Text("%s", buf);
}

void ModelTree::Render(bool* openWindow) {
    ImGui::Begin(ICON_FA6_FOLDER_TREE"  Model Tree", openWindow, ImGuiWindowFlags_NoScrollbar);
    
    static ImGuiTableFlags tableFlags =
        ImGuiTableFlags_Resizable |
        ImGuiTableFlags_NoBordersInBodyUntilResize |
        ImGuiTableFlags_Reorderable |
        ImGuiTableFlags_PadOuterX |
        ImGuiTableFlags_BordersOuter;

    // Mesh Tree 렌더링
    renderMeshTable(tableFlags);
    
    // Crystal Structure 섹션
    renderXsfStructureTable(tableFlags);
    
    // 삭제 확인 팝업 렌더링
    renderDeleteConfirmPopup();
    renderClearMeasurementsConfirmPopup();

    ImGui::End();
}

