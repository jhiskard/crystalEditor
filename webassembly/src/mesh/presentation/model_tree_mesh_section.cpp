#include "../../mesh/presentation/model_tree_panel.h"
#include "../../app.h"
#include "../../shell/presentation/font/font_registry.h"
#include "../../mesh/presentation/mesh_detail_panel.h"
#include "../../mesh/domain/lcrs_tree.h"
#include "../../config/log_config.h"
#include "../../atoms/atoms_template.h"
#include "../application/mesh_command_service.h"
#include "../application/mesh_query_service.h"

#include <imgui.h>

namespace {
void TextColoredCentered(const ImVec4& color, const char* text) {
    float columnWidth = ImGui::GetColumnWidth();
    float textWidth = ImGui::CalcTextSize(text).x;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (columnWidth - textWidth) * 0.5f);
    ImGui::TextColored(color, "%s", text);
}

void TextCenteredInt(int value) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", value);
    float columnWidth = ImGui::GetColumnWidth();
    float textWidth = ImGui::CalcTextSize(buf).x;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (columnWidth - textWidth) * 0.5f);
    ImGui::Text("%s", buf);
}
} // namespace

void ModelTree::renderMeshTable(ImGuiTableFlags tableFlags) {
    constexpr int32_t TABLE_COLUMN_COUNT = 4;
    static const char* tableHeaders[TABLE_COLUMN_COUNT] = { "Name", "Id", "Show", "Remove" };
    static ImGuiTableColumnFlags tableColumnFlags[TABLE_COLUMN_COUNT] = {
        ImGuiTableColumnFlags_WidthStretch,
        ImGuiTableColumnFlags_WidthFixed,
        ImGuiTableColumnFlags_WidthFixed,
        ImGuiTableColumnFlags_WidthFixed
    };
    static float tableColumnSizes[TABLE_COLUMN_COUNT] = { 27.0f, 3.0f, 4.0f, 4.0f };

    bool isOpen = ImGui::CollapsingHeader("Mesh Tree", ImGuiTreeNodeFlags_DefaultOpen);
    if (isOpen && ImGui::BeginTable("MeshTree_Table", TABLE_COLUMN_COUNT, tableFlags)) {
        for (int iCol = 0; iCol < TABLE_COLUMN_COUNT; iCol++) {
            ImGui::TableSetupColumn(tableHeaders[iCol], tableColumnFlags[iCol], App::TextBaseWidth() * tableColumnSizes[iCol]);
        }

        ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
        for (int iCol = 0; iCol < TABLE_COLUMN_COUNT; iCol++) {
            ImGui::TableSetColumnIndex(iCol);
            ImGui::PushID(iCol);
            ImGui::TableHeader(tableHeaders[iCol]);
            if (iCol == 2) {
                App::AddTooltip("Show/Hide Mesh");
            }
            else if (iCol == 3) {
                App::AddTooltip("Delete Mesh");
            }
            ImGui::PopID();
        }

        mesh::application::MeshQueryService& meshQuery = mesh::application::GetMeshQueryService();
        TreeNode* rootNode = meshQuery.MeshTree()->GetRootMutable();
        renderMeshTree(rootNode);

        ImGui::EndTable();
    }

    // Delete mesh 처리 (일반 VTK 메시)
    MeshDetail& meshDetail = MeshDetail::Instance();
    if (m_DeleteMeshId != -1) {
        if (m_DeleteMeshId != 0) {
            mesh::application::GetMeshCommandService().DeleteMesh(m_DeleteMeshId);
        } else {
            SPDLOG_ERROR("Cannot delete the root node.");
        }
        m_DeleteMeshId = -1;
    }
    else {
        // 더블클릭으로 선택 해제
        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsWindowFocused()) {
            m_SelectedMeshId = -1;

            meshDetail.SetHasEdgeMeshColorChanged(false);
            meshDetail.SetHasEdgeMeshOpacityChanged(false);
            meshDetail.SetHasFaceMeshColorChanged(false);
            meshDetail.SetHasFaceMeshOpacityChanged(false);
            meshDetail.SetHasFaceMeshEdgeColorChanged(false);
            meshDetail.SetHasVolumeMeshColorChanged(false);
            meshDetail.SetHasVolumeMeshOpacityChanged(false);
            meshDetail.SetHasVolumeMeshEdgeColorChanged(false);
            meshDetail.SetHasVolumeRenderOpacityMinChanged(false);
            meshDetail.SetHasVolumeRenderOpacityMaxChanged(false);
            meshDetail.SetHasVolumeWindowLevelChanged(false);
            meshDetail.SetHasVolumeColorCurveChanged(false);
            meshDetail.SetHasVolumeSampleDistanceChanged(false);
        }
    }
}


void ModelTree::renderMeshTree(TreeNode* node) {
    if (node == nullptr) {
        return;
    }

    mesh::application::MeshQueryService& meshQuery = mesh::application::GetMeshQueryService();
    mesh::application::MeshCommandService& meshCommand = mesh::application::GetMeshCommandService();
    Mesh* mesh = meshQuery.FindMeshByIdMutable(node->GetId());
    if (mesh == nullptr) {
        return;
    }
    if (node->GetParent() && node->GetParent()->GetId() != 0) {
        Mesh* parentMesh = meshQuery.FindMeshByIdMutable(node->GetParent()->GetId());
        if (parentMesh && parentMesh->IsXsfStructure() && !mesh->IsXsfStructure()) {
            if (node->GetRightSibling()) {
                renderMeshTree(node->GetRightSiblingMutable());
            }
            return;
        }
    }

    ImGuiTreeNodeFlags flagOpen = 
        ImGuiTreeNodeFlags_OpenOnArrow | 
        ImGuiTreeNodeFlags_OpenOnDoubleClick | 
        ImGuiTreeNodeFlags_DefaultOpen | 
        ImGuiTreeNodeFlags_SpanFullWidth;
    ImGuiTreeNodeFlags flagNoOpen = 
        ImGuiTreeNodeFlags_Leaf | 
        ImGuiTreeNodeFlags_NoTreePushOnOpen | 
        ImGuiTreeNodeFlags_Bullet | 
        ImGuiTreeNodeFlags_SpanFullWidth;

    bool isTreeOpen = false;
    bool isXsfStructure = mesh->IsXsfStructure();

    ImGui::TableNextRow();

    // Column 0: Mesh name
    ImGui::TableSetColumnIndex(0);
    std::string meshNodeId = std::to_string(node->GetId());
    ImGuiTreeNodeFlags selectFlag = m_SelectedMeshId == node->GetId() ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None;
    
    if (node->GetLeftChild()) {
        isTreeOpen = ImGui::TreeNodeEx(meshNodeId.c_str(), flagOpen | selectFlag, ICON_FA6_CUBES"  %s", mesh->GetName());
    } else {
        const char* icon = isXsfStructure ? ICON_FA6_ATOM : ICON_FA6_CUBE;
        ImGui::TreeNodeEx(meshNodeId.c_str(), flagNoOpen | selectFlag, "%s  %s", icon, mesh->GetName());
    }

    // Click event
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        if (!mesh) {
            return;
        }

        MeshDetail& meshDetail = MeshDetail::Instance();

        if (m_SelectedMeshId != node->GetId()) {
            m_SelectedMeshId = node->GetId();

            if (!isXsfStructure) {
                meshDetail.SetUiEdgeMeshColor(mesh->GetEdgeMeshColor());
                meshDetail.SetUiEdgeMeshOpacity(mesh->GetEdgeMeshOpacity());
                meshDetail.SetUiEdgeMeshVisibility(mesh->GetEdgeMeshVisibility());

                meshDetail.SetUiFaceMeshColor(mesh->GetFaceMeshColor());
                meshDetail.SetUiFaceMeshOpacity(mesh->GetFaceMeshOpacity());
                meshDetail.SetUiFaceMeshVisibility(mesh->GetFaceMeshVisibility());

                meshDetail.SetUiVolumeMeshColor(mesh->GetVolumeMeshColor());
                meshDetail.SetUiVolumeMeshOpacity(mesh->GetVolumeMeshOpacity());
                meshDetail.SetUiVolumeMeshEdgeColor(mesh->GetVolumeMeshEdgeColor());
                meshDetail.SetUiVolumeMeshVisibility(mesh->GetVolumeMeshVisibility());

                // Volume rendering settings (다른 사람 코드에서 추가)
                meshDetail.SetUiVolumeWindowLevel(mesh->GetVolumeWindow(), mesh->GetVolumeLevel());
                meshDetail.SetUiVolumeRenderOpacityMin(mesh->GetVolumeRenderOpacityMin());
                meshDetail.SetUiVolumeRenderOpacity(mesh->GetVolumeRenderOpacity());
                meshDetail.SetUiVolumeRenderMode(
                    mesh->GetVolumeRenderMode() == Mesh::VolumeRenderMode::Volume ? 1 : 0);
                meshDetail.SetUiVolumeColorPreset(static_cast<int>(mesh->GetVolumeColorPreset()));
                meshDetail.SetUiVolumeColorCurve(mesh->GetVolumeColorMidpoint(), mesh->GetVolumeColorSharpness());
                meshDetail.SetUiVolumeSampleDistanceIndex(mesh->GetVolumeSampleDistanceIndex());
                meshDetail.SetUiVolumeQuality(static_cast<int>(mesh->GetVolumeQuality()));
            }
        }
        else {
            m_SelectedMeshId = -1;

            meshDetail.SetHasEdgeMeshColorChanged(false);
            meshDetail.SetHasEdgeMeshOpacityChanged(false);
            meshDetail.SetHasFaceMeshColorChanged(false);
            meshDetail.SetHasFaceMeshOpacityChanged(false);
            meshDetail.SetHasFaceMeshEdgeColorChanged(false);
            meshDetail.SetHasVolumeMeshColorChanged(false);
            meshDetail.SetHasVolumeMeshOpacityChanged(false);
            meshDetail.SetHasVolumeMeshEdgeColorChanged(false);
            meshDetail.SetHasVolumeRenderOpacityMinChanged(false);
            meshDetail.SetHasVolumeRenderOpacityMaxChanged(false);
            meshDetail.SetHasVolumeWindowLevelChanged(false);
            meshDetail.SetHasVolumeColorCurveChanged(false);
            meshDetail.SetHasVolumeSampleDistanceChanged(false);
        }
    }

    // Column 1: Mesh id
    ImGui::TableSetColumnIndex(1);
    TextCenteredInt(node->GetId());

    // Column 2: Show/Hide mesh icon
    ImGui::TableSetColumnIndex(2);
    if (node->GetId() != 0) {
        ImVec4 curColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);
        
        if (isXsfStructure) {
            AtomsTemplate& atomsTemplate = AtomsTemplate::Instance();
            bool isVisible = atomsTemplate.IsStructureVisible(node->GetId());
            
            if (isVisible) {
                TextColoredCentered(ImVec4(curColor.x, curColor.y, curColor.z, 1.0f), ICON_FA6_EYE);
                if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                    atomsTemplate.SetStructureVisible(node->GetId(), false);
                    meshCommand.HideMesh(node->GetId());
                }
            } else {
                TextColoredCentered(ImVec4(curColor.x, curColor.y, curColor.z, 0.4f), ICON_FA6_EYE_SLASH);
                if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                    atomsTemplate.SetStructureVisible(node->GetId(), true);
                    MeshDetail::Instance().SetUiVolumeMeshVisibility(true);
                    meshCommand.ShowMesh(node->GetId());
                }
            }
        } else {
            if (node->GetIconState() == IconState::VISIBLE) {
                TextColoredCentered(ImVec4(curColor.x, curColor.y, curColor.z, 1.0f), ICON_FA6_EYE);
                if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                    meshCommand.HideMesh(node->GetId());
                }
            } else if (node->GetIconState() == IconState::PARTIAL) {
                TextColoredCentered(ImVec4(curColor.x, curColor.y, curColor.z, 0.4f), ICON_FA6_EYE);
                if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                    meshCommand.ShowMesh(node->GetId());
                }
            } else if (node->GetIconState() == IconState::HIDDEN) {
                TextColoredCentered(ImVec4(curColor.x, curColor.y, curColor.z, 0.4f), ICON_FA6_EYE_SLASH);
                if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                    meshCommand.ShowMesh(node->GetId());
                }
            }
        }
    }

    // Column 3: Delete mesh icon
    ImGui::TableSetColumnIndex(3);
    if (node->GetId() != 0) {
        TextColoredCentered(ImVec4(0.7f, 0.0f, 0.0f, 1.0f), ICON_FA6_TRASH_CAN);
        
        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            m_PendingDeleteMeshId = node->GetId();
            m_ShowDeleteConfirmPopup = true;
        }
    }

    if (isTreeOpen) {
        renderMeshTree(node->GetLeftChildMutable());
        ImGui::TreePop();
    }

    if (node->GetRightSibling()) {
        renderMeshTree(node->GetRightSiblingMutable());
    }
}



