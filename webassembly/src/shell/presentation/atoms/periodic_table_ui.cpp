// webassembly/src/atoms/ui/periodic_table_ui.cpp
#include "periodic_table_ui.h"
#include "ui_color_utils.h"
#include "../../../workspace/runtime/legacy_atoms_runtime.h"
#include "../../../structure/domain/atoms/element_database.h"
#include "../../../structure/domain/atoms/cell_manager.h"
#include "../../../config/log_config.h"
#include <cstdio>

namespace atoms {
namespace ui {

// ============================================================================
// ������
// ============================================================================

PeriodicTableUI::PeriodicTableUI(WorkspaceRuntimeModel* parent)
    : m_parent(parent)
    , m_elementDB(&atoms::domain::ElementDatabase::getInstance())
    , m_selectedElementSymbol("")
    , m_category(0)
    , m_atomPosition{0.0f, 0.0f, 0.0f}
    , m_useFractionalCoords(false) {
    
    SPDLOG_DEBUG("PeriodicTableUI initialized");
}

// ============================================================================
// ���� ������
// ============================================================================

void PeriodicTableUI::render() {
    // ī�װ�� ����
    renderCategoryFilter();
    
    // ���õ� ���� ǥ��
    ImGui::TextWrapped("Click on an element to select it. Selected element: %s", 
                      m_selectedElementSymbol.empty() ? "None" : m_selectedElementSymbol.c_str());
    ImGui::Separator();
    
    // ���� �ֱ���ǥ
    renderMainPeriodicTable();
    
    // ��Ÿ����/��Ƽ����
    if (m_category == 0 || m_category == 9) {
        ImGui::Dummy(ImVec2(1.0f, 10.0f));
        renderLanthanides();
    }
    
    if (m_category == 0 || m_category == 10) {
        ImGui::Dummy(ImVec2(1.0f, 10.0f));
        renderActinides();
    }
    
    // ���õ� ���� �� ����
    if (!m_selectedElementSymbol.empty()) {
        renderSelectedElementDetails();
    }
}

// ============================================================================
// ī�װ�� ����
// ============================================================================

void PeriodicTableUI::renderCategoryFilter() {
    const char* categoryNames[] = {
        "All Elements",
        "Non-metals",
        "Alkali Metals",
        "Alkaline Earth Metals",
        "Transition Metals",
        "Post-transition Metals",
        "Metalloid",
        "Halogens",
        "Noble Gases",
        "Lanthanide",
        "Actinide"
    };
    
    if (ImGui::BeginCombo("Classification", categoryNames[m_category])) {
        for (int i = 0; i < 11; i++) {
            if (ImGui::Selectable(categoryNames[i], m_category == i)) {
                m_category = i;
                SPDLOG_DEBUG("Category changed to: {}", categoryNames[i]);
            }
        }
        ImGui::EndCombo();
    }
}

// ============================================================================
// ���� �ֱ���ǥ ������
// ============================================================================

void PeriodicTableUI::renderMainPeriodicTable() {
    // â �ʺ� �°� ������ ����
    const float availWidth = ImGui::GetContentRegionAvail().x;
    const float scale = availWidth / TOTAL_WIDTH;
    const float scaledButtonSize = BUTTON_SIZE * scale;
    const float scaledSpacing = SPACING * scale;
    
    // ��Ÿ�� ����
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(scaledSpacing, scaledSpacing));
    
    // �ֱ���ǥ ��ȸ (1-7�ֱ�, 1-18��)
    for (int period = 1; period <= 7; period++) {
        for (int group = 1; group <= 18; group++) {
            // �ش� ��ġ�� ���� ã��
            const atoms::domain::ElementInfo* element = nullptr;
            
            auto allSymbols = m_elementDB->getAllSymbols();
            for (const auto& symbol : allSymbols) {
                auto pos = m_elementDB->getElementPosition(symbol);
                if (pos.period == period && pos.group == group) {
                    element = m_elementDB->getElementInfo(symbol);
                    break;
                }
            }
            
            // ���� �ٿ� ��ġ
            if (group > 1) {
                ImGui::SameLine();
            }
            
            if (element && shouldShowElement(*element)) {
                // ���� ��ư ������
                renderElementButton(*element, scaledButtonSize);
            } else {
                // �� ���� �Ǵ� ���͸��� ����
                ImGui::Dummy(ImVec2(scaledButtonSize, scaledButtonSize));
            }
        }
    }
    
    ImGui::PopStyleVar(); // ItemSpacing
}

// ============================================================================
// ��Ÿ���� ������
// ============================================================================

void PeriodicTableUI::renderLanthanides() {
    ImGui::Text("Lanthanides:");
    
    // â �ʺ� �°� ������ ����
    const float availWidth = ImGui::GetContentRegionAvail().x;
    const float scale = availWidth / TOTAL_WIDTH;
    const float scaledButtonSize = BUTTON_SIZE * scale;
    const float scaledSpacing = SPACING * scale;
    
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(scaledSpacing, scaledSpacing));
    
    // Period 8, Group 3-17
    for (int group = 3; group <= 17; group++) {
        const atoms::domain::ElementInfo* element = nullptr;
        
        auto allSymbols = m_elementDB->getAllSymbols();
        for (const auto& symbol : allSymbols) {
            auto pos = m_elementDB->getElementPosition(symbol);
            if (pos.period == 8 && pos.group == group) {
                element = m_elementDB->getElementInfo(symbol);
                break;
            }
        }
        
        if (group > 3) {
            ImGui::SameLine();
        }
        
        if (element) {
            renderElementButton(*element, scaledButtonSize);
        } else {
            ImGui::Dummy(ImVec2(scaledButtonSize, scaledButtonSize));
        }
    }
    
    ImGui::PopStyleVar();
}

// ============================================================================
// ��Ƽ���� ������
// ============================================================================

void PeriodicTableUI::renderActinides() {
    ImGui::Text("Actinides:");
    
    // â �ʺ� �°� ������ ����
    const float availWidth = ImGui::GetContentRegionAvail().x;
    const float scale = availWidth / TOTAL_WIDTH;
    const float scaledButtonSize = BUTTON_SIZE * scale;
    const float scaledSpacing = SPACING * scale;
    
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(scaledSpacing, scaledSpacing));
    
    // Period 9, Group 3-17
    for (int group = 3; group <= 17; group++) {
        const atoms::domain::ElementInfo* element = nullptr;
        
        auto allSymbols = m_elementDB->getAllSymbols();
        for (const auto& symbol : allSymbols) {
            auto pos = m_elementDB->getElementPosition(symbol);
            if (pos.period == 9 && pos.group == group) {
                element = m_elementDB->getElementInfo(symbol);
                break;
            }
        }
        
        if (group > 3) {
            ImGui::SameLine();
        }
        
        if (element) {
            renderElementButton(*element, scaledButtonSize);
        } else {
            ImGui::Dummy(ImVec2(scaledButtonSize, scaledButtonSize));
        }
    }
    
    ImGui::PopStyleVar();
}

// ============================================================================
// ���� ���� ��ư ������
// ============================================================================

void PeriodicTableUI::renderElementButton(const atoms::domain::ElementInfo& element, 
                                          float buttonSize) {
    // ���� ������ ���ÿ� Ǫ���� ���� ����
    int pushedColors = 0;
    int pushedVars = 0;
    
    // ȣ�� �� ��Ƽ�� ���� ���
    // ImVec4 hoveredColor = calculateHoveredColor(element.defaultColor);
    // ImVec4 activeColor = calculateActiveColor(element.defaultColor);
    // ImVec4 textColor = GetContrastTextColor(element.defaultColor);
    ImVec4 baseColor = atoms::ui::ToImVec4(element.defaultColor);
    ImVec4 hoveredColor = calculateHoveredColor(baseColor);
    ImVec4 activeColor = calculateActiveColor(baseColor);
    ImVec4 textColor = GetContrastTextColor(baseColor);

    // ���� ��ư ��Ÿ�� ����
    // ImGui::PushStyleColor(ImGuiCol_Button, element.defaultColor);
    ImGui::PushStyleColor(ImGuiCol_Button, baseColor);
    pushedColors++;
    
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoveredColor);
    pushedColors++;
    
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, activeColor);
    pushedColors++;
    
    ImGui::PushStyleColor(ImGuiCol_Text, textColor);
    pushedColors++;
    
    // ���õ� ���Ҵ� �׵θ��� ǥ��
    if (element.symbol == m_selectedElementSymbol) {
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        pushedColors++;
        
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
        pushedVars++;
    }
    
    // ��ư ����
    char buttonLabel[16];
    snprintf(buttonLabel, sizeof(buttonLabel), "%s\n%d", 
             element.symbol.c_str(), element.atomicNumber);
    
    if (ImGui::Button(buttonLabel, ImVec2(buttonSize, buttonSize))) {
        m_selectedElementSymbol = element.symbol;
        SPDLOG_DEBUG("Selected element: {}", element.symbol);
    }
    
    // ���콺 ȣ�� �� ���� ǥ��
    if (ImGui::IsItemHovered()) {
        renderElementTooltip(element);
    }
    
    // ��Ÿ�� ���� ����
    if (pushedColors > 0) {
        ImGui::PopStyleColor(pushedColors);
    }
    if (pushedVars > 0) {
        ImGui::PopStyleVar(pushedVars);
    }
}

// ============================================================================
// ���� ���� ������
// ============================================================================

void PeriodicTableUI::renderElementTooltip(const atoms::domain::ElementInfo& element) {
    // ������ ���� ��Ÿ�� ����
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    
    ImGui::BeginTooltip();
    ImGui::Text("%s (%s)", element.name.c_str(), element.symbol.c_str());
    ImGui::Text("Atomic Number: %d", element.atomicNumber);
    ImGui::Text("Atomic Mass: %.4f", element.atomicMass);
    ImGui::Text("Covalent Radius: %.2f A", element.covalentRadius);
    ImGui::Text("Group: %d, Period: %d", element.groupNumber, element.period);
    ImGui::Text("Classification: %s", element.classification.c_str());
    ImGui::EndTooltip();
    
    ImGui::PopStyleColor(); // ������ ���� Pop
}

// ============================================================================
// ���õ� ���� �� ����
// ============================================================================

void PeriodicTableUI::renderSelectedElementDetails() {
    const atoms::domain::ElementInfo* element = m_elementDB->getElementInfo(m_selectedElementSymbol);
    if (!element) {
        return;
    }
    
    ImGui::Separator();
    ImGui::Text("Atom Position");
    
    // Cell�� ���� ��� üũ�ڽ� ��Ȱ��ȭ
    // bool hasCellInfo = !cellEdgeActors.empty();
    bool hasCellInfo = m_parent->hasUnitCell();
    // bool hasCellInfo = true;
    
    if (!hasCellInfo) {
        ImGui::BeginDisabled();
    }
    
    // ���� ��ǥ ��� ���� üũ�ڽ�
    ImGui::Checkbox("Use Fractional Coordinates##PeriodicTable", &m_useFractionalCoords);
    
    // üũ�ڽ� ����
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && !hasCellInfo) {
        ImGui::BeginTooltip();
        ImGui::Text("Fractional coordinates require a unit cell.");
        ImGui::Text("Please create or import a cell first.");
        ImGui::EndTooltip();
    } else if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text("Fractional coordinates are relative to the unit cell axes");
        ImGui::Text("(values from 0 to 1)");
        ImGui::EndTooltip();
    }
    
    if (!hasCellInfo) {
        ImGui::EndDisabled();
    }
    
    // ��ǥ �Է� �� �߰� ��ư
    if (m_useFractionalCoords && hasCellInfo) {
        static float fracPosition[3] = {0.0f, 0.0f, 0.0f};
        
        ImGui::DragFloat3("Fractional (a,b,c)", fracPosition, 0.01f, 0.0f, 1.0f);
        
        if (ImGui::Button("Add to Structure")) {
            // Fractional ��ǥ�� Cartesian ��ǥ�� ��ȯ
            float cartPosition[3];
            atoms::domain::fractionalToCartesian(fracPosition, cartPosition, cellInfo.matrix);
            
            // ���� �߰�
            // (const char* symbol, const ImVec4& color, float radius, 
            // const float position[3], atoms::domain::AtomType atomType)
            m_parent->createAtomSphere(
                element->symbol.c_str(),
                element->defaultColor,
                element->covalentRadius,
                cartPosition
            );
            
            SPDLOG_INFO("Added {} atom at fractional position ({:.3f}, {:.3f}, {:.3f})",
                       element->symbol, fracPosition[0], fracPosition[1], fracPosition[2]);
        }
    } else {
        // ���� ��ǥ �Է�
        ImGui::DragFloat3("Position (X,Y,Z)", m_atomPosition, 0.1f);
        
        if (ImGui::Button("Add to Structure")) {
            // ���� �߰�
            m_parent->createAtomSphere(
                element->symbol.c_str(),
                element->defaultColor,
                element->covalentRadius,
                m_atomPosition
            );
            
            SPDLOG_INFO("Added {} atom at cartesian position ({:.3f}, {:.3f}, {:.3f})",
                       element->symbol, m_atomPosition[0], m_atomPosition[1], m_atomPosition[2]);
        }
    }
}

// ============================================================================
// ��ƿ��Ƽ �޼���
// ============================================================================

bool PeriodicTableUI::shouldShowElement(const atoms::domain::ElementInfo& element) const {
    if (m_category == 0) {
        return true; // All Elements
    }
    
    const char* targetClassification = nullptr;
    
    switch (m_category) {
        case 1: targetClassification = "Non-metals"; break;
        case 2: targetClassification = "Alkali Metals"; break;
        case 3: targetClassification = "Alkaline Earth Metals"; break;
        case 4: targetClassification = "Transition Metals"; break;
        case 5: targetClassification = "Post-transition Metals"; break;
        case 6: targetClassification = "Metalloid"; break;
        case 7: targetClassification = "Halogens"; break;
        case 8: targetClassification = "Noble Gases"; break;
        case 9: targetClassification = "Lanthanide"; break;
        case 10: targetClassification = "Actinide"; break;
        default: return true;
    }
    
    return element.classification == targetClassification;
}

ImVec4 PeriodicTableUI::calculateHoveredColor(const ImVec4& baseColor) const {
    return ImVec4(
        baseColor.x * 1.2f,
        baseColor.y * 1.2f,
        baseColor.z * 1.2f,
        baseColor.w
    );
}

ImVec4 PeriodicTableUI::calculateActiveColor(const ImVec4& baseColor) const {
    return ImVec4(
        baseColor.x * 0.8f,
        baseColor.y * 0.8f,
        baseColor.z * 0.8f,
        baseColor.w
    );
}

// ============================================================================
// Getter �޼���
// ============================================================================

std::string PeriodicTableUI::getSelectedElementSymbol() const {
    return m_selectedElementSymbol;
}

void PeriodicTableUI::clearSelection() {
    m_selectedElementSymbol = "";
    SPDLOG_DEBUG("Cleared element selection");
}

} // namespace ui
} // namespace atoms








