#include "model_tree.h"
#include "shell/runtime/workbench_runtime.h"
#include "font_manager.h"
#include "workspace/application/workspace_command_service.h"

// ImGui
#include <imgui.h>

ModelTree& ModelTree::Instance() {
    return GetWorkbenchRuntime().ModelTreePanel();
}

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
    workspace::application::GetWorkspaceCommandService().SetSelectedMeshId(m_SelectedMeshId);

    ImGui::End();
}


