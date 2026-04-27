// webassembly/src/atoms/ui/atom_editor_ui.cpp
#include "atom_editor_ui.h"
#include "ui_color_utils.h"
#include "../../../workspace/runtime/legacy_atoms_runtime.h"
#include "../../../structure/domain/atoms/atom_manager.h"
#include "../../../structure/domain/atoms/bond_manager.h"
#include "../../../structure/domain/atoms/cell_manager.h"
#include "../../../structure/domain/structure_repository.h"
#include "../../../config/log_config.h"
#include <algorithm>
#include <unordered_map>

namespace atoms {
namespace ui {

namespace {
auto& createdAtoms = structure::domain::GetStructureRepository().CreatedAtoms();
auto& surroundingAtoms = structure::domain::GetStructureRepository().SurroundingAtoms();
auto& createdBonds = structure::domain::GetStructureRepository().CreatedBonds();
auto& surroundingBonds = structure::domain::GetStructureRepository().SurroundingBonds();
auto& atomGroups = structure::domain::GetStructureRepository().AtomGroups();
}

AtomEditorUI::AtomEditorUI(WorkspaceRuntimeModel* parent)
    : m_parent(parent) {
    SPDLOG_DEBUG("AtomEditorUI initialized");
}

void AtomEditorUI::render() {
    // ���� ��� ���� (static���� ����)
    static bool editMode = false;
    static bool hasChanges = false; // ������� ����
    static bool useFractionalCoords = false;

    // �⺻ ���� ǥ��
    ImGui::Text("Total atoms: %d", (int)createdAtoms.size());
    ImGui::Text("Total bonds: %d", (int)createdBonds.size());

    ImGui::Separator();

    // Fractional coordinates üũ�ڽ� �߰�
    if (ImGui::Checkbox("Fractional coordinates", &useFractionalCoords)) {
        SPDLOG_DEBUG("Fractional coordinates mode changed to: {}", useFractionalCoords ? "enabled" : "disabled");
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Show coordinates as fractional (a, b, c) instead of Cartesian (X, Y, Z)");
    }

    ImGui::SameLine();
        
    // Boundary atoms üũ�ڽ�
    // UI ���´� WorkspaceRuntimeModel::isSurroundingsVisible()�� �׻� ����ȭ
    if (m_parent) {
        m_boundaryAtomsEnabled = m_parent->isSurroundingsVisible();
    }
    bool boundaryAtomsEnabled = m_boundaryAtomsEnabled;
    if (ImGui::Checkbox("Boundary atoms", &boundaryAtomsEnabled)) {
        m_boundaryAtomsEnabled = boundaryAtomsEnabled;
        if (m_parent) {
            SPDLOG_INFO(
                "{} boundary atoms visualization",
                boundaryAtomsEnabled ? "Enabling" : "Disabling");
            m_parent->SetBoundaryAtomsEnabled(boundaryAtomsEnabled);
        }
    }

    // ���� �ؽ�Ʈ
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Show/hide atoms at unit cell boundaries\nand their bonds with original atoms");
    }

    ImGui::SameLine();
    
    // Edit mode üũ�ڽ� �߰�
    bool previousEditMode = editMode;
    if (ImGui::Checkbox("Edit mode##atomEdit", &editMode)) {
        if (!editMode && previousEditMode && hasChanges) {
            // Edit mode ���� �� ������� ���� �� ������Ʈ
            SPDLOG_INFO("Exiting edit mode - applying changes and updating rendering");
            
            // ?? ������: applyAtomChanges()�� ���ο��� BatchGuard ���
            // UI �����忡�� ���� ȣ�� ����
            if (m_parent) {
                m_parent->ApplyAtomChangesFromEditor();
            }
            hasChanges = false;
            
        } else if (editMode && !previousEditMode) {
            SPDLOG_DEBUG("Entering edit mode");
        }
    }
    if (ImGui::IsItemHovered()) {
        if (editMode) {
            ImGui::SetTooltip("Exit edit mode and apply changes\n(atoms and bonds will be updated)");
        } else {
            ImGui::SetTooltip("Enter edit mode to modify atom symbols and positions");
        }
    }
    
    ImGui::Separator();
    
    // ?? ������: ��ġ ��� ���� ǥ�� �߰� (����� �ǵ��)
    if (m_parent && m_parent->isBatchMode()) {
        ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "? Batch mode active - updates optimized");
        ImGui::Separator();
    }
    
    // ���� ���̺� ������ - editMode�� useFractionalCoords�� ����
    bool tableHasChanges = renderAtomTable(editMode, useFractionalCoords);
    if (tableHasChanges) {
        hasChanges = true;
    }
}

bool AtomEditorUI::renderAtomTable(bool editMode, bool useFractionalCoords) {
    static std::vector<atoms::domain::AtomInfo> lastDeletedAtoms;

    // ?? ���� ���� ����Ʈ ���� (ORIGINAL + SURROUNDING)
    std::vector<std::pair<atoms::domain::AtomInfo*, bool>> allAtoms; // pair<����������, ��������>
    
    // ���� ���ڵ� �߰�
    for (auto& atom : createdAtoms) {
        allAtoms.push_back({&atom, true});
    }
    
    // �ֺ� ���ڵ� �߰� (surroundingsVisible�� ����)
    if (m_parent && m_parent->isSurroundingsVisible()) {
        for (auto& atom : surroundingAtoms) {
            allAtoms.push_back({&atom, false});
        }
    }
    
    if (allAtoms.empty()) {
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "No atoms created yet.");
        return false;
    }
    
    // ?? ���� ���� ID �� �ε��� ���� ���� (SURROUNDING ���� ǥ�ÿ�)
    std::map<uint32_t, size_t> originalIdToIndex;
    for (size_t i = 0; i < createdAtoms.size(); ++i) {
        if (createdAtoms[i].id != 0) {
            originalIdToIndex[createdAtoms[i].id] = i + 1; // 1-based �ε���
        }
    }
    
    bool hasChanges = false;

    ImGuiTableFlags tableFlags =
        ImGuiTableFlags_SizingFixedFit | 
        ImGuiTableFlags_Reorderable | 
        ImGuiTableFlags_Borders |
        ImGuiTableFlags_RowBg;

    ImGuiTableFlags columnFlags = ImGuiTableColumnFlags_WidthStretch;
    
    // ���̺� ���� - ���� ������ ���� ũ�� ����
    ImVec2 tableSize = ImVec2(0.0f, 0.0f);
    if (allAtoms.size() > 20) {
        tableSize = ImVec2(0.0f, 1000.0f);
        tableFlags |= ImGuiTableFlags_ScrollY;  
    }
    
    // ?? �÷� �� ���� - Radius �÷� �߰�
    int columnCount = editMode ? 9 : 8; // Edit ���: Select + Edit + Radius �÷� ǥ��
    
    if (ImGui::BeginTable("AtomsTable", columnCount, tableFlags, tableSize)) {
        
        // ?? ���̺� ��� ���� - Edit ��忡���� Select �÷� ����
        ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Select", ImGuiTableColumnFlags_WidthFixed); // �׻� ǥ��
        
        if (editMode) {
            ImGui::TableSetupColumn("Edit", ImGuiTableColumnFlags_WidthFixed); // Edit ��忡���� �߰�
        }
        
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Symbol", ImGuiTableColumnFlags_WidthFixed);
        
        if (useFractionalCoords) {
            ImGui::TableSetupColumn("a", columnFlags);
            ImGui::TableSetupColumn("b", columnFlags);
            ImGui::TableSetupColumn("c", columnFlags);
        } else {
            ImGui::TableSetupColumn("X", columnFlags);
            ImGui::TableSetupColumn("Y", columnFlags);
            ImGui::TableSetupColumn("Z", columnFlags);
        }
        ImGui::TableSetupColumn("Radius", columnFlags);
        
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();
        
        // ?? ���յ� ���� ������ ������
        for (size_t i = 0; i < allAtoms.size(); ++i) {
            atoms::domain::AtomInfo* atom = allAtoms[i].first;
            bool isOriginal = allAtoms[i].second;
            
            ImGui::TableNextRow();
            
            // 1. �Ϸù�ȣ (#)
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%zu", i + 1);
            
            // 2. ���� üũ�ڽ� (Select) - �׻� ǥ��
            ImGui::TableSetColumnIndex(1);
            ImGui::PushID(static_cast<int>(i * 2)); // Edit �÷��� ID �浹 ����
            
            // SURROUNDING ���ڴ� ���� �Ұ�
            if (isOriginal) {
                bool selected = atom->selected;
                if (ImGui::Checkbox("##select", &selected)) {
                    atom->selected = selected;
                    if (m_parent) {
                        m_parent->SyncCreatedAtomSelectionVisualsFromState();
                    }
                    SPDLOG_DEBUG("Atom {} ({}) selection changed to: {}", 
                               i + 1, atom->symbol, selected ? "selected" : "deselected");
                }
            } else {
                // SURROUNDING ���ڴ� ��Ȱ��ȭ�� üũ�ڽ�
                ImGui::BeginDisabled();
                bool dummySelected = false;
                ImGui::Checkbox("##select_disabled", &dummySelected);
                ImGui::EndDisabled();
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Surrounding atoms cannot be selected");
                }
            }
            ImGui::PopID();
            
            // ?? 3. ���� ǥ�� (Edit) - Edit ��忡���� ǥ��
            int nextColumnIndex = 2;
            if (editMode) {
                ImGui::TableSetColumnIndex(2);
                ImGui::PushID(static_cast<int>(i * 2 + 1)); // Select �÷��� ID �浹 ����
                
                if (isOriginal && atom->modified) {
                    ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.0f, 1.0f), "MOD");
                } else if (isOriginal) {
                    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "---");
                } else {
                    // SURROUNDING ���ڴ� ���� �Ұ� ǥ��
                    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "N/A");
                }
                ImGui::PopID();
                nextColumnIndex = 3;
            }
            
            // 4 (�Ǵ� 3). Type �÷� (���� Ÿ�� ����)
            ImGui::TableSetColumnIndex(nextColumnIndex);
            if (atom->atomType == atoms::domain::AtomType::ORIGINAL) {
                ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "ORIG");
            } else {
                ImGui::TextColored(ImVec4(0.8f, 0.6f, 0.2f, 1.0f), "SURR");
            }
            
            // ID ������ �������� ǥ�� (���� ���� ���� ����)
            if (ImGui::IsItemHovered()) {
                std::string tooltipText = "Atom ID: " + std::to_string(atom->id) + 
                                        "\nInstance Index: " + std::to_string(atom->instanceIndex) + 
                                        "\nInstanced: " + (atom->isInstanced ? "Yes" : "No") +
                                        "\nEditable: " + (isOriginal ? "Yes" : "No");
                
                // ?? SURROUNDING ������ ��� ���� ���� ���� �߰�
                if (!isOriginal && atom->originalAtomId != 0) {
                    auto it = originalIdToIndex.find(atom->originalAtomId);
                    if (it != originalIdToIndex.end()) {
                        tooltipText += "\nDerived from: Original Atom #" + std::to_string(it->second) + 
                                      " (ID: " + std::to_string(atom->originalAtomId) + ")";
                    } else {
                        tooltipText += "\nDerived from: Original Atom ID " + std::to_string(atom->originalAtomId) + 
                                      " (not found in current list)";
                    }
                }
                
                ImGui::SetTooltip("%s", tooltipText.c_str());
            }
            
            // 5 (�Ǵ� 4). ���� ��ȣ (Symbol) - ?? SURROUNDING ���ڴ� ���� ID ���� ǥ��
            ImGui::TableSetColumnIndex(nextColumnIndex + 1);
            
            // SURROUNDING ���ڴ� ������ �ణ ��Ӱ� ǥ��
            // ImVec4 bgColor = atom->color;
            ImVec4 bgColor = atoms::ui::ToImVec4(atom->color);
            if (!isOriginal) {
                // SURROUNDING ���ڴ� ������ �� ��Ӱ�
                bgColor.x *= 0.7f;
                bgColor.y *= 0.7f;
                bgColor.z *= 0.7f;
            }
            bgColor.w = 0.3f;
            // ImVec4 textColor = GetContrastTextColor(atom->color);
            ImVec4 textColor = GetContrastTextColor(bgColor);
            
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::ColorConvertFloat4ToU32(bgColor));
            
            if (!editMode || !isOriginal) {
                // �б� ���� ��� �Ǵ� SURROUNDING ����
                if (isOriginal) {
                    // ORIGINAL ����: �Ϲ� ǥ��
                    ImGui::TextColored(textColor, "%s", atom->symbol.c_str());
                } else {
                    // ?? SURROUNDING ����: Symbol(OriginalID) �������� ǥ��
                    std::string displayText;
                    if (atom->originalAtomId != 0) {
                        auto it = originalIdToIndex.find(atom->originalAtomId);
                        if (it != originalIdToIndex.end()) {
                            // ���� ���ڸ� ã�� ���: Symbol(#Index)
                            displayText = atom->symbol + "(#" + std::to_string(it->second) + ")";
                        } else {
                            // ���� ���ڸ� ã�� ���� ���: Symbol(ID:XXX)
                            displayText = atom->symbol + "(ID:" + std::to_string(atom->originalAtomId) + ")";
                        }
                    } else {
                        // originalAtomId�� �������� ���� ���: Symbol(?)
                        displayText = atom->symbol + "(?)";
                    }
                    
                    ImGui::TextColored(textColor, "%s", displayText.c_str());
                    
                    // ?? SURROUNDING ���� Symbol�� ���콺 ���� �� �� ���� ǥ��
                    if (ImGui::IsItemHovered()) {
                        if (atom->originalAtomId != 0) {
                            auto it = originalIdToIndex.find(atom->originalAtomId);
                            if (it != originalIdToIndex.end()) {
                                ImGui::SetTooltip("Surrounding atom derived from:\nOriginal Atom #%zu (ID: %u)", 
                                                it->second, atom->originalAtomId);
                            } else {
                                ImGui::SetTooltip("Surrounding atom derived from:\nOriginal Atom ID %u (not in current list)", 
                                                atom->originalAtomId);
                            }
                        } else {
                            ImGui::SetTooltip("Surrounding atom with unknown origin\n(originalAtomId not set)");
                        }
                    }
                }
            } else {
                // ���� ��� (ORIGINAL ���ڸ�)
                char symbolBuffer[16];
                strncpy(symbolBuffer, atom->tempSymbol.c_str(), sizeof(symbolBuffer) - 1);
                symbolBuffer[sizeof(symbolBuffer) - 1] = '\0';
                
                ImGui::SetNextItemWidth(-1);
                ImGui::PushID(static_cast<int>(i * 10 + 5)); // Symbol InputText ���� ID
                if (ImGui::InputText("##symbol", symbolBuffer, sizeof(symbolBuffer))) {
                    atom->tempSymbol = std::string(symbolBuffer);
                    atom->modified = true;
                    hasChanges = true;
                    SPDLOG_DEBUG("Atom {} symbol changed to: {}", i + 1, atom->tempSymbol);
                }
                ImGui::PopID();
            }
            
            // ?? ��ǥ ǥ�� �� ���� (�÷� �ε���: nextColumnIndex + 2, 3, 4)
            float* coords;
            float* tempCoords;
            
            if (useFractionalCoords) {
                coords = const_cast<float*>(atom->fracPosition);
                tempCoords = atom->tempFracPosition;
            } else {
                coords = const_cast<float*>(atom->position);
                tempCoords = atom->tempPosition;
            }
            
            // ù ��° ��ǥ (X �Ǵ� a)
            ImGui::TableSetColumnIndex(nextColumnIndex + 2);
            if (!editMode || !isOriginal) {
                ImGui::Text("%.4f", coords[0]);
            } else {
                ImGui::SetNextItemWidth(-1);
                ImGui::PushID(static_cast<int>(i * 10 + 6)); // Coord0 InputFloat ���� ID
                if (ImGui::InputFloat("##coord0", &tempCoords[0], 0.01f, 0.1f, "%.4f")) {
                    atom->modified = true;
                    hasChanges = true;
                    SPDLOG_DEBUG("Atom {} coordinate[0] changed to: {:.4f}", i + 1, tempCoords[0]);
                    
                    if (useFractionalCoords) {
                        atoms::domain::fractionalToCartesian(atom->tempFracPosition, atom->tempPosition, cellInfo.matrix);
                    } else {
                        atoms::domain::cartesianToFractional(atom->tempPosition, atom->tempFracPosition, cellInfo.invmatrix);
                    }
                }
                ImGui::PopID();
            }
            
            // �� ��° ��ǥ (Y �Ǵ� b)
            ImGui::TableSetColumnIndex(nextColumnIndex + 3);
            if (!editMode || !isOriginal) {
                ImGui::Text("%.4f", coords[1]);
            } else {
                ImGui::SetNextItemWidth(-1);
                ImGui::PushID(static_cast<int>(i * 10 + 7)); // Coord1 InputFloat ���� ID
                if (ImGui::InputFloat("##coord1", &tempCoords[1], 0.01f, 0.1f, "%.4f")) {
                    atom->modified = true;
                    hasChanges = true;
                    SPDLOG_DEBUG("Atom {} coordinate[1] changed to: {:.4f}", i + 1, tempCoords[1]);
                    
                    if (useFractionalCoords) {
                        atoms::domain::fractionalToCartesian(atom->tempFracPosition, atom->tempPosition, cellInfo.matrix);
                    } else {
                        atoms::domain::cartesianToFractional(atom->tempPosition, atom->tempFracPosition, cellInfo.invmatrix);
                    }
                }
                ImGui::PopID();
            }
            
            // �� ��° ��ǥ (Z �Ǵ� c)
            ImGui::TableSetColumnIndex(nextColumnIndex + 4);
            if (!editMode || !isOriginal) {
                ImGui::Text("%.4f", coords[2]);
            } else {
                ImGui::SetNextItemWidth(-1);
                ImGui::PushID(static_cast<int>(i * 10 + 8)); // Coord2 InputFloat ���� ID
                if (ImGui::InputFloat("##coord2", &tempCoords[2], 0.01f, 0.1f, "%.4f")) {
                    atom->modified = true;
                    hasChanges = true;
                    SPDLOG_DEBUG("Atom {} coordinate[2] changed to: {:.4f}", i + 1, tempCoords[2]);
                    
                    if (useFractionalCoords) {
                        atoms::domain::fractionalToCartesian(atom->tempFracPosition, atom->tempPosition, cellInfo.matrix);
                    } else {
                        atoms::domain::cartesianToFractional(atom->tempPosition, atom->tempFracPosition, cellInfo.invmatrix);
                    }
                }
                ImGui::PopID();
            }

            // Radius �÷� (Z/c ��)
            ImGui::TableSetColumnIndex(nextColumnIndex + 5);
            if (!editMode || !isOriginal) {
                ImGui::Text("%.4f", atom->radius);
            } else {
                ImGui::SetNextItemWidth(-1);
                ImGui::PushID(static_cast<int>(i * 10 + 9)); // Radius InputFloat ���� ID
                float tempRadius = atom->tempRadius;
                if (ImGui::InputFloat("##radius", &tempRadius, 0.01f, 0.1f, "%.4f")) {
                    const float clampedRadius = std::max(tempRadius, 0.001f);
                    if (clampedRadius != atom->tempRadius) {
                        atom->tempRadius = clampedRadius;
                        atom->modified = true;
                        hasChanges = true;
                        SPDLOG_DEBUG("Atom {} radius changed to: {:.4f}", i + 1, atom->tempRadius);
                    }
                }
                ImGui::PopID();
            }
        }
        
        ImGui::EndTable();
    }

    renderSelectionPanel(editMode, useFractionalCoords, &hasChanges);
    
    // ?? ���̺� �ϴ� ���� - ���յ� ��� ǥ�� (���� ���� ���� ����)
    ImGui::Text("Showing %zu atoms", allAtoms.size());
    
    // Ÿ�Ժ� ���� ��� �� ǥ��
    int originalCount = 0, surroundingCount = 0;
    int trackedSurrounding = 0, untrackedSurrounding = 0;
    
    for (const auto& [atom, isOriginal] : allAtoms) {
        if (atom->atomType == atoms::domain::AtomType::ORIGINAL) {
            originalCount++;
        } else {
            surroundingCount++;
            // ?? SURROUNDING ������ ���� ���� ���� Ȯ��
            if (atom->originalAtomId != 0) {
                auto it = originalIdToIndex.find(atom->originalAtomId);
                if (it != originalIdToIndex.end()) {
                    trackedSurrounding++;
                } else {
                    untrackedSurrounding++;
                }
            } else {
                untrackedSurrounding++;
            }
        }
    }
    
    if (surroundingCount > 0) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "(%d ORIG", originalCount);
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.8f, 0.6f, 0.2f, 1.0f), "+ %d SURR)", surroundingCount);
        
        // ?? SURROUNDING ������ ���� ���� ǥ��
        if (trackedSurrounding > 0 || untrackedSurrounding > 0) {
            ImGui::SameLine();
            if (trackedSurrounding > 0) {
                ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "[%d tracked", trackedSurrounding);
                if (untrackedSurrounding > 0) {
                    ImGui::SameLine();
                    ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), ", %d untracked]", untrackedSurrounding);
                } else {
                    ImGui::SameLine();
                    ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "]");
                }
            } else {
                ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "[%d untracked]", untrackedSurrounding);
            }
        }
    }
    
    if (allAtoms.size() > 20) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 1.0f, 1.0f), "(Scrollable)");
    }
    
    // ?? ��� ���� ǥ�� - Edit ��忡���� ���� ��� ���
    ImGui::SameLine();
    if (editMode) {
        ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.0f, 1.0f), "[Edit + Select Mode]");
        if (hasChanges) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "*");
        }
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 1.0f, 1.0f), "(ORIG only)");
    } else {
        if (useFractionalCoords) {
            ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "[Fractional]");
        } else {
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "[Cartesian]");
        }
    }

// void AtomEditorUI::renderEditPanel() {

    // ���õ� ���� ���� ǥ�� (ORIGINAL ���ڸ� ���) - Edit ��忡���� ǥ��
    ImGui::Separator();
    
    std::vector<size_t> selectedAtomIndices;
    std::map<std::string, int> selectedElementCounts;
    
    for (size_t i = 0; i < allAtoms.size(); ++i) {
        atoms::domain::AtomInfo* atom = allAtoms[i].first;
        bool isOriginal = allAtoms[i].second;
        
        // ORIGINAL ���ڸ� ���� �����ϹǷ� ORIGINAL ���ڸ� ���
        if (isOriginal && atom->selected) {
            selectedAtomIndices.push_back(i + 1);
            selectedElementCounts[atom->symbol]++;
        }
    }

    if (!selectedAtomIndices.empty()) {
        ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "Selected atoms: %zu (ORIG only)", selectedAtomIndices.size());
        
        if (selectedAtomIndices.size() < 20) {
            ImGui::TextWrapped("Atom indices: ");
            ImGui::SameLine();
            
            std::string indicesText = "";
            for (size_t i = 0; i < selectedAtomIndices.size(); ++i) {
                if (i > 0) indicesText += ", ";
                indicesText += std::to_string(selectedAtomIndices[i]);
            }
            
            ImGui::TextWrapped("%s", indicesText.c_str());
        } else {
            ImGui::TextWrapped("Selected by element: ");
            
            std::string elementCountsText = "";
            bool first = true;
            for (const auto& pair : selectedElementCounts) {
                if (!first) elementCountsText += ", ";
                elementCountsText += pair.first + "(" + std::to_string(pair.second) + ")";
                first = false;
            }
            
            ImGui::TextWrapped("%s", elementCountsText.c_str());
        }
    }


    bool hasSelectedAtoms = !selectedAtomIndices.empty();
    ImGui::BeginDisabled(!hasSelectedAtoms);
    if (ImGui::Button("Delete Selected")) {
        if (m_parent) {
            lastDeletedAtoms.clear();
            for (const auto& atom : createdAtoms) {
                if (atom.selected) {
                    lastDeletedAtoms.push_back(atom);
                }
            }

            bool wasSurroundingsVisible = m_parent->isSurroundingsVisible();
            if (wasSurroundingsVisible) {
                m_parent->hideSurroundingAtoms();
            }

            WorkspaceRuntimeModel::BatchGuard guard = m_parent->createBatchGuard();

            for (const auto& atom : createdAtoms) {
                if (atom.selected && atom.id != 0) {
                    removeAtomFromGroup(atom.symbol, atom.id);
                }
            }

            createdAtoms.erase(
                std::remove_if(createdAtoms.begin(), createdAtoms.end(),
                               [](const atoms::domain::AtomInfo& atom) { return atom.selected; }),
                createdAtoms.end());

            std::unordered_map<uint32_t, size_t> idToIndex;
            idToIndex.reserve(createdAtoms.size());
            for (size_t i = 0; i < createdAtoms.size(); ++i) {
                idToIndex[createdAtoms[i].id] = i;
            }

            for (auto& [symbol, group] : atomGroups) {
                for (size_t i = 0; i < group.atomIds.size(); ++i) {
                    if (group.atomTypes[i] != atoms::domain::AtomType::ORIGINAL) {
                        continue;
                    }
                    auto it = idToIndex.find(group.atomIds[i]);
                    if (it != idToIndex.end()) {
                        group.originalIndices[i] = it->second;
                    }
                }
            }

            m_parent->clearAllBonds();
            if (wasSurroundingsVisible) {
                m_parent->createSurroundingAtoms();
            } else if (!createdAtoms.empty()) {
                m_parent->createAllBonds();
            }
        }
    }
    ImGui::EndDisabled();

    ImGui::SameLine();

    bool hasRestoreData = !lastDeletedAtoms.empty();
    ImGui::BeginDisabled(!hasRestoreData);
    if (ImGui::Button("Restore Deleted")) {
        if (m_parent) {
            bool wasSurroundingsVisible = m_parent->isSurroundingsVisible();
            if (wasSurroundingsVisible) {
                m_parent->hideSurroundingAtoms();
            }

            WorkspaceRuntimeModel::BatchGuard guard = m_parent->createBatchGuard();

            for (const auto& atom : lastDeletedAtoms) {
                m_parent->createAtomSphere(
                    atom.symbol.c_str(),
                    atom.color,
                    atom.radius,
                    atom.position,
                    atoms::domain::AtomType::ORIGINAL
                );
            }
            lastDeletedAtoms.clear();

            m_parent->clearAllBonds();
            if (wasSurroundingsVisible) {
                m_parent->createSurroundingAtoms();
            } else if (!createdAtoms.empty()) {
                m_parent->createAllBonds();
            }
        }
    }
    ImGui::EndDisabled();

    if (!hasSelectedAtoms) {
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "No atoms selected");
    }
    
    // ?? SURROUNDING ���� ���� ǥ�� (���� ���� ���� ����)
    if (surroundingCount > 0) {
        ImGui::TextColored(ImVec4(0.8f, 0.6f, 0.2f, 1.0f), "Surrounding atoms: %d (view only)", surroundingCount);
        if (trackedSurrounding > 0) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "- %d with origin tracking", trackedSurrounding);
        }
        if (untrackedSurrounding > 0) {
            if (trackedSurrounding > 0) {
                ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "- %d without origin tracking", untrackedSurrounding);
            } else {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "- %d without origin tracking", untrackedSurrounding);
            }
        }
    }
    
    // Edit ��忡�� ���� ���� ���� �߰� ǥ��
    if (editMode) {
        ImGui::Separator();
        
        int modifiedCount = 0;
        for (const auto& [atom, isOriginal] : allAtoms) {
            if (isOriginal && atom->modified) {
                modifiedCount++;
            }
        }
        
        if (modifiedCount > 0) {
            ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.0f, 1.0f), "Modified atoms: %d (ORIG only)", modifiedCount);
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 1.0f, 1.0f), "Uncheck 'Edit mode' to apply changes");
        } else {
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "No modifications made");
        }
        
        if (surroundingCount > 0) {
            ImGui::TextColored(ImVec4(0.8f, 0.6f, 0.2f, 1.0f), "Note: %d surrounding atoms are read-only", surroundingCount);
            if (untrackedSurrounding > 0) {
                ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "Warning: %d surrounding atoms have unknown origin", untrackedSurrounding);
            }
        }
    }
    
    return hasChanges;
}

void AtomEditorUI::renderSelectionPanel(bool editMode, bool useFractionalCoords, bool* hasChanges) {
    if (createdAtoms.empty()) {
        return;
    }

    std::map<std::string, std::pair<int, int>> elementStats;
    int selectedCount = 0;
    for (const auto& atom : createdAtoms) {
        auto& stats = elementStats[atom.symbol];
        stats.first += 1;
        if (atom.selected) {
            stats.second += 1;
            selectedCount += 1;
        }
    }

    if (elementStats.empty()) {
        return;
    }

    ImGui::Separator();
    ImGui::Text("Select by element");
    ImGui::Spacing();

    if (ImGui::Button("Select all")) {
        for (auto& atom : createdAtoms) {
            atom.selected = true;
        }
        if (m_parent) {
            m_parent->SyncCreatedAtomSelectionVisualsFromState();
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Select none")) {
        for (auto& atom : createdAtoms) {
            atom.selected = false;
        }
        if (m_parent) {
            m_parent->SyncCreatedAtomSelectionVisualsFromState();
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Invert selection")) {
        for (auto& atom : createdAtoms) {
            atom.selected = !atom.selected;
        }
        if (m_parent) {
            m_parent->SyncCreatedAtomSelectionVisualsFromState();
        }
    }
    ImGui::Spacing();

    int index = 0;
    for (const auto& [symbol, counts] : elementStats) {
        if (index > 0) {
            ImGui::SameLine();
        }

        bool checked = counts.second == counts.first;

        ImGui::PushID(symbol.c_str());
        if (ImGui::Checkbox(symbol.c_str(), &checked)) {
            for (auto& atom : createdAtoms) {
                if (atom.symbol == symbol) {
                    atom.selected = checked;
                }
            }
            if (m_parent) {
                m_parent->SyncCreatedAtomSelectionVisualsFromState();
            }
        }

        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Total: %d, Selected: %d", counts.first, counts.second);
        }
        ImGui::PopID();

        index += 1;
    }

    ImGui::Separator();
    ImGui::Text("Translate selected");
    ImGui::Spacing();

    // if (selectedCount == 0) {
    //     ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "No atoms selected.");
    // }

    bool canTranslate = selectedCount > 0;
    ImGui::BeginDisabled(!canTranslate);

    static float translateDelta[3] = {0.0f, 0.0f, 0.0f};
    const char* label = useFractionalCoords ? "Delta (a,b,c)" : "Delta (X,Y,Z)";
    ImGui::InputFloat3(label, translateDelta, "%.4f");

    if (ImGui::Button("Apply translation")) {
        for (auto& atom : createdAtoms) {
            if (!atom.selected) {
                continue;
            }

            if (useFractionalCoords) {
                atom.tempFracPosition[0] += translateDelta[0];
                atom.tempFracPosition[1] += translateDelta[1];
                atom.tempFracPosition[2] += translateDelta[2];
                atoms::domain::fractionalToCartesian(atom.tempFracPosition,
                                                     atom.tempPosition,
                                                     cellInfo.matrix);
            } else {
                atom.tempPosition[0] += translateDelta[0];
                atom.tempPosition[1] += translateDelta[1];
                atom.tempPosition[2] += translateDelta[2];
                atoms::domain::cartesianToFractional(atom.tempPosition,
                                                     atom.tempFracPosition,
                                                     cellInfo.invmatrix);
            }

            atom.modified = true;
        }
        if (m_parent) {
            m_parent->ApplyAtomChangesFromEditor();
        }
        if (hasChanges) {
            *hasChanges = false;
        }
    }

    ImGui::EndDisabled();

    ImGui::Separator();
    ImGui::Text("Set radius (selected) as");
    ImGui::Spacing();

    static float selectedRadiusValue = 1.000f;
    constexpr float kMinSelectedRadius = 0.300f;
    constexpr float kMaxSelectedRadius = 5.000f;
    const bool hasSelectedForRadius = selectedCount > 0;

    ImGui::BeginDisabled(!hasSelectedForRadius);
    const ImGuiStyle& style = ImGui::GetStyle();
    const float applyRadiusButtonWidth =
        ImGui::CalcTextSize("Apply radius").x + style.FramePadding.x * 2.0f;
    float radiusInputWidth =
        ImGui::GetContentRegionAvail().x - applyRadiusButtonWidth - style.ItemSpacing.x;
    radiusInputWidth = std::max(radiusInputWidth, 120.0f);
    ImGui::SetNextItemWidth(radiusInputWidth);
    ImGui::InputFloat("##selectedRadiusValue", &selectedRadiusValue, 0.01f, 0.1f, "%.3f");
    ImGui::SameLine();

    const bool radiusInValidRange =
        selectedRadiusValue >= kMinSelectedRadius &&
        selectedRadiusValue <= kMaxSelectedRadius;

    ImGui::BeginDisabled(!radiusInValidRange);
    if (ImGui::Button("Apply radius")) {
        bool anySelected = false;
        for (auto& atom : createdAtoms) {
            if (!atom.selected) {
                continue;
            }
            atom.tempRadius = selectedRadiusValue;
            atom.modified = true;
            anySelected = true;
        }

        if (anySelected && m_parent) {
            m_parent->ApplyAtomChangesFromEditor();
        }
        if (anySelected && hasChanges) {
            *hasChanges = false;
        }
    }
    ImGui::EndDisabled();
    ImGui::EndDisabled();

    if (!radiusInValidRange) {
        ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "Valid range: 0.3 ~ 5.0");
    }
}

} // ui
} // atoms










