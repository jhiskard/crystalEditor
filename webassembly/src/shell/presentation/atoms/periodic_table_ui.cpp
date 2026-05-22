// webassembly/src/atoms/ui/periodic_table_ui.cpp
#include "periodic_table_ui.h"
#include "ui_color_utils.h"
#include "../../../workspace/runtime/workspace_runtime_model_ref.h"
#include "../../../structure/domain/atoms/element_database.h"
#include "../../../structure/domain/atoms/cell_manager.h"
#include "../../../config/log_config.h"
#include <cstdio>

namespace atoms {
namespace ui {

// ============================================================================
// 占쏙옙占쏙옙占쏙옙
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
// 占쏙옙占쏙옙 占쏙옙占쏙옙占쏙옙
// ============================================================================

void PeriodicTableUI::render() {
    // 카占쌓곤옙占?占쏙옙占쏙옙
    renderCategoryFilter();
    
    // 占쏙옙占시듸옙 占쏙옙占쏙옙 표占쏙옙
    ImGui::TextWrapped("Click on an element to select it. Selected element: %s", 
                      m_selectedElementSymbol.empty() ? "None" : m_selectedElementSymbol.c_str());
    ImGui::Separator();
    
    // 占쏙옙占쏙옙 占쌍깍옙占쏙옙표
    renderMainPeriodicTable();
    
    // 占쏙옙타占쏙옙占쏙옙/占쏙옙티占쏙옙占쏙옙
    if (m_category == 0 || m_category == 9) {
        ImGui::Dummy(ImVec2(1.0f, 10.0f));
        renderLanthanides();
    }
    
    if (m_category == 0 || m_category == 10) {
        ImGui::Dummy(ImVec2(1.0f, 10.0f));
        renderActinides();
    }
    
    // 占쏙옙占시듸옙 占쏙옙占쏙옙 占쏙옙 占쏙옙占쏙옙
    if (!m_selectedElementSymbol.empty()) {
        renderSelectedElementDetails();
    }
}

// ============================================================================
// 카占쌓곤옙占?占쏙옙占쏙옙
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
// 占쏙옙占쏙옙 占쌍깍옙占쏙옙표 占쏙옙占쏙옙占쏙옙
// ============================================================================

void PeriodicTableUI::renderMainPeriodicTable() {
    // 창 占십븝옙 占승곤옙 占쏙옙占쏙옙占쏙옙 占쏙옙占쏙옙
    const float availWidth = ImGui::GetContentRegionAvail().x;
    const float scale = availWidth / TOTAL_WIDTH;
    const float scaledButtonSize = BUTTON_SIZE * scale;
    const float scaledSpacing = SPACING * scale;
    
    // 占쏙옙타占쏙옙 占쏙옙占쏙옙
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(scaledSpacing, scaledSpacing));
    
    // 占쌍깍옙占쏙옙표 占쏙옙회 (1-7占쌍깍옙, 1-18占쏙옙)
    for (int period = 1; period <= 7; period++) {
        for (int group = 1; group <= 18; group++) {
            // 占쌔댐옙 占쏙옙치占쏙옙 占쏙옙占쏙옙 찾占쏙옙
            const atoms::domain::ElementInfo* element = nullptr;
            
            auto allSymbols = m_elementDB->getAllSymbols();
            for (const auto& symbol : allSymbols) {
                auto pos = m_elementDB->getElementPosition(symbol);
                if (pos.period == period && pos.group == group) {
                    element = m_elementDB->getElementInfo(symbol);
                    break;
                }
            }
            
            // 占쏙옙占쏙옙 占쌕울옙 占쏙옙치
            if (group > 1) {
                ImGui::SameLine();
            }
            
            if (element && shouldShowElement(*element)) {
                // 占쏙옙占쏙옙 占쏙옙튼 占쏙옙占쏙옙占쏙옙
                renderElementButton(*element, scaledButtonSize);
            } else {
                // 占쏙옙 占쏙옙占쏙옙 占실댐옙 占쏙옙占싶몌옙占쏙옙 占쏙옙占쏙옙
                ImGui::Dummy(ImVec2(scaledButtonSize, scaledButtonSize));
            }
        }
    }
    
    ImGui::PopStyleVar(); // ItemSpacing
}

// ============================================================================
// 占쏙옙타占쏙옙占쏙옙 占쏙옙占쏙옙占쏙옙
// ============================================================================

void PeriodicTableUI::renderLanthanides() {
    ImGui::Text("Lanthanides:");
    
    // 창 占십븝옙 占승곤옙 占쏙옙占쏙옙占쏙옙 占쏙옙占쏙옙
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
// 占쏙옙티占쏙옙占쏙옙 占쏙옙占쏙옙占쏙옙
// ============================================================================

void PeriodicTableUI::renderActinides() {
    ImGui::Text("Actinides:");
    
    // 창 占십븝옙 占승곤옙 占쏙옙占쏙옙占쏙옙 占쏙옙占쏙옙
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
// 占쏙옙占쏙옙 占쏙옙占쏙옙 占쏙옙튼 占쏙옙占쏙옙占쏙옙
// ============================================================================

void PeriodicTableUI::renderElementButton(const atoms::domain::ElementInfo& element, 
                                          float buttonSize) {
    // 占쏙옙占쏙옙 占쏙옙占쏙옙占쏙옙 占쏙옙占시울옙 푸占쏙옙占쏙옙 占쏙옙占쏙옙 占쏙옙占쏙옙
    int pushedColors = 0;
    int pushedVars = 0;
    
    // 호占쏙옙 占쏙옙 占쏙옙티占쏙옙 占쏙옙占쏙옙 占쏙옙占?
    // ImVec4 hoveredColor = calculateHoveredColor(element.defaultColor);
    // ImVec4 activeColor = calculateActiveColor(element.defaultColor);
    // ImVec4 textColor = GetContrastTextColor(element.defaultColor);
    ImVec4 baseColor = atoms::ui::ToImVec4(element.defaultColor);
    ImVec4 hoveredColor = calculateHoveredColor(baseColor);
    ImVec4 activeColor = calculateActiveColor(baseColor);
    ImVec4 textColor = GetContrastTextColor(baseColor);

    // 占쏙옙占쏙옙 占쏙옙튼 占쏙옙타占쏙옙 占쏙옙占쏙옙
    // ImGui::PushStyleColor(ImGuiCol_Button, element.defaultColor);
    ImGui::PushStyleColor(ImGuiCol_Button, baseColor);
    pushedColors++;
    
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoveredColor);
    pushedColors++;
    
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, activeColor);
    pushedColors++;
    
    ImGui::PushStyleColor(ImGuiCol_Text, textColor);
    pushedColors++;
    
    // 占쏙옙占시듸옙 占쏙옙占쌀댐옙 占쌓두몌옙占쏙옙 표占쏙옙
    if (element.symbol == m_selectedElementSymbol) {
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        pushedColors++;
        
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
        pushedVars++;
    }
    
    // 占쏙옙튼 占쏙옙占쏙옙
    char buttonLabel[16];
    snprintf(buttonLabel, sizeof(buttonLabel), "%s\n%d", 
             element.symbol.c_str(), element.atomicNumber);
    
    if (ImGui::Button(buttonLabel, ImVec2(buttonSize, buttonSize))) {
        m_selectedElementSymbol = element.symbol;
        SPDLOG_DEBUG("Selected element: {}", element.symbol);
    }
    
    // 占쏙옙占쎌스 호占쏙옙 占쏙옙 占쏙옙占쏙옙 표占쏙옙
    if (ImGui::IsItemHovered()) {
        renderElementTooltip(element);
    }
    
    // 占쏙옙타占쏙옙 占쏙옙占쏙옙 占쏙옙占쏙옙
    if (pushedColors > 0) {
        ImGui::PopStyleColor(pushedColors);
    }
    if (pushedVars > 0) {
        ImGui::PopStyleVar(pushedVars);
    }
}

// ============================================================================
// 占쏙옙占쏙옙 占쏙옙占쏙옙 占쏙옙占쏙옙占쏙옙
// ============================================================================

void PeriodicTableUI::renderElementTooltip(const atoms::domain::ElementInfo& element) {
    // 占쏙옙占쏙옙占쏙옙 占쏙옙占쏙옙 占쏙옙타占쏙옙 占쏙옙占쏙옙
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    
    ImGui::BeginTooltip();
    ImGui::Text("%s (%s)", element.name.c_str(), element.symbol.c_str());
    ImGui::Text("Atomic Number: %d", element.atomicNumber);
    ImGui::Text("Atomic Mass: %.4f", element.atomicMass);
    ImGui::Text("Covalent Radius: %.2f A", element.covalentRadius);
    ImGui::Text("Group: %d, Period: %d", element.groupNumber, element.period);
    ImGui::Text("Classification: %s", element.classification.c_str());
    ImGui::EndTooltip();
    
    ImGui::PopStyleColor(); // 占쏙옙占쏙옙占쏙옙 占쏙옙占쏙옙 Pop
}

// ============================================================================
// 占쏙옙占시듸옙 占쏙옙占쏙옙 占쏙옙 占쏙옙占쏙옙
// ============================================================================

void PeriodicTableUI::renderSelectedElementDetails() {
    const atoms::domain::ElementInfo* element = m_elementDB->getElementInfo(m_selectedElementSymbol);
    if (!element) {
        return;
    }
    
    ImGui::Separator();
    ImGui::Text("Atom Position");
    
    // Cell占쏙옙 占쏙옙占쏙옙 占쏙옙占?체크占쌘쏙옙 占쏙옙활占쏙옙화
    // bool hasCellInfo = !cellEdgeActors.empty();
    bool hasCellInfo = m_parent->hasUnitCell();
    // bool hasCellInfo = true;
    
    if (!hasCellInfo) {
        ImGui::BeginDisabled();
    }
    
    // 占쏙옙占쏙옙 占쏙옙표 占쏙옙占?占쏙옙占쏙옙 체크占쌘쏙옙
    ImGui::Checkbox("Use Fractional Coordinates##PeriodicTable", &m_useFractionalCoords);
    
    // 체크占쌘쏙옙 占쏙옙占쏙옙
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
    
    // 占쏙옙표 占쌉뤄옙 占쏙옙 占쌩곤옙 占쏙옙튼
    if (m_useFractionalCoords && hasCellInfo) {
        static float fracPosition[3] = {0.0f, 0.0f, 0.0f};
        
        ImGui::DragFloat3("Fractional (a,b,c)", fracPosition, 0.01f, 0.0f, 1.0f);
        
        if (ImGui::Button("Add to Structure")) {
            // Fractional 占쏙옙표占쏙옙 Cartesian 占쏙옙표占쏙옙 占쏙옙환
            float cartPosition[3];
            atoms::domain::fractionalToCartesian(fracPosition, cartPosition, cellInfo.matrix);
            
            // 占쏙옙占쏙옙 占쌩곤옙
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
        // 占쏙옙占쏙옙 占쏙옙표 占쌉뤄옙
        ImGui::DragFloat3("Position (X,Y,Z)", m_atomPosition, 0.1f);
        
        if (ImGui::Button("Add to Structure")) {
            // 占쏙옙占쏙옙 占쌩곤옙
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
// 占쏙옙틸占쏙옙티 占쌨쇽옙占쏙옙
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
// Getter 占쌨쇽옙占쏙옙
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








