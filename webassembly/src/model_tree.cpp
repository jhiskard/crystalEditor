#include "model_tree.h"
#include "font_manager.h"
#include "workspace/application/workspace_command_service.h"

// ImGui
#include <imgui.h>

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
    workspace::application::GetWorkspaceCommandService().SetSelectedMeshId(s_SelectedMeshId);

    ImGui::End();
}

