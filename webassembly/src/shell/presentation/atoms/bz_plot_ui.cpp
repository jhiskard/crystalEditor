#include "bz_plot_ui.h"
#include "../../../workspace/runtime/workspace_runtime_model_ref.h"
#include "../../../structure/domain/atoms/special_points.h"
#include "../../../structure/domain/atoms/cell_manager.h"
#include "../../../app.h"
#include "../../../config/log_config.h"
#include "../../../render/application/render_gateway.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace {
constexpr float kCellMatrixEpsilon = 1e-6f;

bool isSameCellMatrix(
    const std::array<std::array<float, 3>, 3>& lhs,
    const float rhs[3][3]) {
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            if (std::fabs(lhs[static_cast<size_t>(row)][static_cast<size_t>(col)] - rhs[row][col]) >
                kCellMatrixEpsilon) {
                return false;
            }
        }
    }
    return true;
}

void copyCellMatrix(
    std::array<std::array<float, 3>, 3>& dst,
    const float src[3][3]) {
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            dst[static_cast<size_t>(row)][static_cast<size_t>(col)] = src[row][col];
        }
    }
}
} // namespace

namespace atoms {
namespace ui {

BZPlotUI::BZPlotUI(WorkspaceRuntimeModel* parent)
    : m_parent(parent)
{
    SPDLOG_DEBUG("BZPlotUI initialized");
}

void BZPlotUI::SetShowingBZ(bool showing) {
    m_showingBZ = showing;
    if (!m_showingBZ) {
        m_lastErrorMessage.clear();
    }
}

void BZPlotUI::render() {
    if (!m_parent) {
        ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f),
                           "BZPlotUI: WorkspaceRuntimeModel is not available.");
        return;
    }

    renderBandpathConfig();
    ImGui::Spacing();
    renderOptions();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    renderToggleAndClearButtons();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    renderStatus();
    ImGui::Spacing();
    renderSpecialPointsTable();
    // renderBZplot();
}

void BZPlotUI::renderBandpathConfig() {
    ImGui::Text("Bandpath Configuration:");
    ImGui::Spacing();

    // Path 占쌉뤄옙
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Path:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(250.0f);
    ImGui::InputText("##path", m_pathInput, IM_ARRAYSIZE(m_pathInput));
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Enter k-point path using special point symbols\n"
            "Examples:\n"
            "  - All (automatic path selection based on lattice)\n"
            "  - GXMGRX (cubic)\n"
            "  - GXMGRX,MR (with branch)\n"
            "  - GKLUWLK (hexagonal)"
        );
    }

    // Npoints 占쌉뤄옙
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Npoints:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(240.0f);
    if (ImGui::InputInt("##npoints", &m_npointsInput)) {
        if (m_npointsInput < 10)  m_npointsInput = 10;
        if (m_npointsInput > 500) m_npointsInput = 500;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Number of interpolation points along each path segment.\n"
            "Typical range: 10 - 200."
        );
    }
}

void BZPlotUI::renderOptions() {
    ImGui::Text("Options:");
    ImGui::Spacing();

    // ?? 占쏙옙占?占쏙옙占쏙옙 占쏙옙占쏙옙 占쏙옙占쏙옙
    bool prevShowVectors = m_showVectors;
    bool prevShowLabels  = m_showLabels;

    // ?? 占쏙옙占쏙옙 표占쏙옙 占쏙옙占?
    ImGui::Checkbox("Show reciprocal vectors", &m_showVectors);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Toggle visibility of reciprocal lattice vectors.");
    }

    // ?? 占쏙옙 표占쏙옙 占쏙옙占?
    ImGui::Checkbox("Show special point labels", &m_showLabels);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Toggle labels for special k-points.");
    }

    // ?? 占쏙옙 占쏙옙 占싹놂옙占쏙옙 占쏙옙占쏙옙 占쌕뀐옙占쏙옙占쏙옙占?확占쏙옙
    bool vectorsChanged = (prevShowVectors != m_showVectors);
    bool labelsChanged  = (prevShowLabels  != m_showLabels);
    bool optionsChanged = vectorsChanged || labelsChanged;

    // 占싱뱄옙 BZ Plot 占쏙옙占쏙옙占?占쏙옙占쏙옙 占쏙옙占?占쏙옙占쏙옙占쏙옙트
    if (optionsChanged && m_showingBZ && m_parent) {
        m_lastErrorMessage.clear();

        std::string pathStr(m_pathInput);
        int npoints = m_npointsInput;

        // ?? 占쌩울옙: 占쏙옙占쏙옙 占심쇽옙(m_showVectors / m_showLabels)占쏙옙 占쌓댐옙占?占싼겨쇽옙
        // 占쏙옙체 BZ Plot占쏙옙 占쌕쏙옙 占쏙옙占쏙옙占싹듸옙占쏙옙 占싼댐옙.
        bool success = m_parent->EnterBZPlotMode(
            pathStr,
            npoints,
            m_showVectors,   // 占쏙옙 占쏙옙占쏙옙 표占쏙옙 占쏙옙占쏙옙
            m_showLabels,    // 占쏙옙 占쏙옙 표占쏙옙 占쏙옙占쏙옙 (占쏙옙占썩가 占쌕쏙옙)
            m_lastErrorMessage
        );

        // 占쌕쏙옙 占쌓몌옙占쏙옙 占쏙옙占쏙옙 占쏙옙占싸울옙 占쏙옙占쏙옙 占싸그몌옙 占쏙옙占쏙옙占쏙옙占쏙옙占?占쏙옙占?
        if (!success && !m_lastErrorMessage.empty()) {
            SPDLOG_ERROR("Failed to update BZ options: {}", m_lastErrorMessage);
        }
    }
}

void BZPlotUI::renderToggleAndClearButtons() {
    const char* buttonText = m_showingBZ ? "Show Crystal" : "Show BZ Plot";
    const ImGuiStyle& style = ImGui::GetStyle();
    const float maxLabelWidth = std::max(
        std::max(ImGui::CalcTextSize("Show BZ Plot").x, ImGui::CalcTextSize("Show Crystal").x),
        ImGui::CalcTextSize("Clear BZ").x);
    const float minButtonWidth =
        maxLabelWidth + style.FramePadding.x * 2.0f + (App::UiScale() * 12.0f);
    const float availableWidth = ImGui::GetContentRegionAvail().x;
    const bool sameLineButtons =
        availableWidth >= (minButtonWidth * 2.0f + style.ItemSpacing.x);
    const float buttonWidth = sameLineButtons
        ? std::max(minButtonWidth, (availableWidth - style.ItemSpacing.x) * 0.5f)
        : availableWidth;
    const ImVec2 buttonSize(buttonWidth, 0.0f);

    // Show BZ Plot / Show Crystal 占쏙옙占?占쏙옙튼
    if (ImGui::Button(buttonText, buttonSize)) {
        m_lastErrorMessage.clear();

        if (!m_showingBZ) {
            // BZ Plot 占쏙옙占?占쏙옙占쏙옙
            std::string pathStr(m_pathInput);
            int npoints = m_npointsInput;

            bool success = false;
            if (m_parent) {
                success = m_parent->EnterBZPlotMode(
                    pathStr,
                    npoints,
                    m_showVectors,
                    m_showLabels,
                    m_lastErrorMessage
                );
            }

            if (success) {
                m_showingBZ = true;
            }
        } else {
            // Crystal 占쏙옙占쏙옙 占쏙옙占쏙옙
            if (m_parent) {
                m_parent->ExitBZPlotMode();
            }
            m_showingBZ = false;
        }
    }

    if (sameLineButtons) {
        ImGui::SameLine();
    }

    // Clear BZ 占쏙옙튼
    if (ImGui::Button("Clear BZ", buttonSize)) {
        m_lastErrorMessage.clear();
        if (m_parent) {
            // BZ Plot占쏙옙 占쏙옙占쏙옙占싹곤옙 crystal 占쏙옙占쏙옙 占쏙옙占쏙옙
            m_parent->ExitBZPlotMode();
        }
        m_showingBZ = false;
    }
}

void BZPlotUI::renderStatus() {
    ImGui::Spacing();

    if (m_showingBZ) {
        ImGui::TextColored(ImVec4(0.0f, 0.5f, 1.0f, 1.0f),
                           "占쏙옙 BZ Plot Mode");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                           "(vectors: %s, labels: %s)",
                           m_showVectors ? "ON" : "OFF",
                           m_showLabels ? "ON" : "OFF");
    } else {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f),
                           "占쏙옙 Crystal Mode");
    }

    if (!m_lastErrorMessage.empty()) {
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
        ImGui::TextWrapped("Error: %s", m_lastErrorMessage.c_str());
        ImGui::PopStyleColor();
    }
}

void BZPlotUI::renderSpecialPointsTable() {
    // 占쏙옙占쏙옙 타占쏙옙 占쏙옙占쏙옙 占쏙옙 특占쏙옙占쏙옙 占쏙옙占쏙옙占쏙옙 占쏙옙회
    refreshSpecialPointsCache();

    ImGui::Spacing();
    ImGui::Text("Special k-points for lattice: %s", m_cachedLatticeType.c_str());
    ImGui::Spacing();

    if (m_cachedSpecialPoints.empty()) {
        ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.0f, 1.0f),
                           "No special points to display");
        return;
    }

    ImGui::Text("Total points: %zu", m_cachedSpecialPoints.size());
    ImGui::Separator();
    ImGui::Spacing();

    ImGuiTableFlags tableFlags =
        ImGuiTableFlags_Borders |
        ImGuiTableFlags_RowBg |
        ImGuiTableFlags_SizingFixedFit;

    if (ImGui::BeginTable("SpecialPointsTable", 4, tableFlags)) {
        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 60.0f);
        ImGui::TableSetupColumn("a", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableSetupColumn("b", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableSetupColumn("c", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableHeadersRow();

        auto labelColor = ImVec4(0.8f, 0.9f, 1.0f, 1.0f);

        // for (const auto& [label, coords] : m_cachedSpecialPoints)
        for (const auto& kv : m_cachedSpecialPoints) {
            const std::string& label = kv.first;
            const auto& coords = kv.second;

            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::TextColored(labelColor, "%s", label.c_str());

            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%.6f", coords[0]);

            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%.6f", coords[1]);

            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%.6f", coords[2]);
        }

        ImGui::EndTable();
    }
}

void BZPlotUI::refreshSpecialPointsCache() {
    if (m_hasLatticeCache && isSameCellMatrix(m_cachedCellMatrix, cellInfo.matrix)) {
        return;
    }

    copyCellMatrix(m_cachedCellMatrix, cellInfo.matrix);
    m_cachedLatticeType = atoms::domain::SpecialPointsDatabase::detectLatticeType(cellInfo.matrix);
    m_cachedSpecialPoints = atoms::domain::SpecialPointsDatabase::getSpecialPoints(m_cachedLatticeType);
    m_hasLatticeCache = true;
}

/*
// 占쏙옙占쏙옙 static 占쏙옙占쏙옙 占쏙옙占쏙옙占쏙옙占쏙옙 占쏙옙占쏙옙占?占싱듸옙
char  m_pathInput[128] = "All";
int   m_npointsInput   = 50;
bool  m_showVectors    = true;
bool  m_showLabels     = true;
bool  m_showingBZ      = false;
*/
void BZPlotUI::renderBZplot() {
    // 占쌉뤄옙 占십듸옙 占쏙옙占쏙옙 (static占쏙옙占쏙옙 占쏙옙占쏙옙)
    static char pathInput[128] = "All";  // ? 占썩본占쏙옙 "All"占쏙옙 占쏙옙占쏙옙
    static int npointsInput = 50;
    static bool showVectors = true;
    static bool showLabels = true;
    static bool showingBZ = false;  // BZ 표占쏙옙 占쏙옙占쏙옙
    static std::string lastErrorMessage;
    
    // ========================================================================
    // 1. 占쌉뤄옙 UI
    // ========================================================================
    
    ImGui::Text("Bandpath Configuration:");
    ImGui::Spacing();
    
    // Path 占쌉뤄옙
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Path:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(200);
    ImGui::InputText("##path", pathInput, IM_ARRAYSIZE(pathInput));
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Enter k-point path using special point symbols\n"
            "Examples:\n"
            "  - All (automatic path selection based on lattice)\n"
            "  - GXMGRX (cubic)\n"
            "  - GXMGRX,MR (with branch)\n"
            "  - GKLUWLK (hexagonal)"
        );
    }
    
    // Npoints 占쌉뤄옙
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Npoints:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(200);
    ImGui::InputInt("##npoints", &npointsInput);
    if (npointsInput < 10) npointsInput = 10;
    if (npointsInput > 500) npointsInput = 500;
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Number of interpolation points along the path (10-500)");
    }
    
    ImGui::Spacing();
    
    // 占심쇽옙 체크占쌘쏙옙
    ImGui::Checkbox("Show reciprocal vectors", &showVectors);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Display the reciprocal lattice vectors (b1, b2, b3)");
    }
    
    ImGui::Checkbox("Show special point labels", &showLabels);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Display labels at high-symmetry points (占쏙옙, X, M, etc.)");
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // ========================================================================
    // 2. 占쏙옙占?占쏙옙튼 (Show BZ Plot 占쏙옙 Show Crystal)
    // ========================================================================
    
    const char* buttonText = showingBZ ? "Show Crystal" : "Show BZ Plot";
    const ImGuiStyle& style = ImGui::GetStyle();
    const float maxLabelWidth = std::max(
        std::max(ImGui::CalcTextSize("Show BZ Plot").x, ImGui::CalcTextSize("Show Crystal").x),
        ImGui::CalcTextSize("Clear BZ").x);
    const float minButtonWidth =
        maxLabelWidth + style.FramePadding.x * 2.0f + (App::UiScale() * 12.0f);
    const float availableWidth = ImGui::GetContentRegionAvail().x;
    const bool sameLineButtons =
        availableWidth >= (minButtonWidth * 2.0f + style.ItemSpacing.x);
    const float buttonWidth = sameLineButtons
        ? std::max(minButtonWidth, (availableWidth - style.ItemSpacing.x) * 0.5f)
        : availableWidth;
    const ImVec2 buttonSize(buttonWidth, 0.0f);
    
    if (ImGui::Button(buttonText, buttonSize)) {
        if (!showingBZ) {
            // ============================================================
            // BZ Plot 표占쏙옙 占쏙옙占쏙옙 占쏙옙환
            // ============================================================
            SPDLOG_INFO("=== Switching to BZ Plot mode ===");
            SPDLOG_INFO("Path: '{}', npoints: {}, vectors: {}, labels: {}", 
                       pathInput, npointsInput, showVectors, showLabels);
            
            lastErrorMessage.clear();
            
            try {
                // 1. 占쏙옙占쏙옙占쏙옙 占쏙옙占?占쌔븝옙
                double cell[3][3];
                for (int i = 0; i < 3; i++) {
                    for (int j = 0; j < 3; j++) {
                        cell[i][j] = cellInfo.matrix[i][j];
                    }
                }
                double icell[3][3];
                for (int i = 0; i < 3; i++) {
                    for (int j = 0; j < 3; j++) {
                        // icell[i][j] = cellInfo.invmatrix[i][j];
                        // cellInfo.invmatrix is TRANSPOSED reciprocal lattice vectors...
                        icell[i][j] = cellInfo.invmatrix[j][i];
                    }
                }
                
                SPDLOG_DEBUG("Inverse cell matrix:");
                SPDLOG_DEBUG("  [{}, {}, {}]", icell[0][0], icell[0][1], icell[0][2]);
                SPDLOG_DEBUG("  [{}, {}, {}]", icell[1][0], icell[1][1], icell[1][2]);
                SPDLOG_DEBUG("  [{}, {}, {}]", icell[2][0], icell[2][1], icell[2][2]);
                
                // 2. BZ vertices 占쏙옙占?
                SPDLOG_INFO("Calculating BZ vertices...");
                auto bzData = atoms::domain::BZCalculator::calculateBZVertices(icell);
                
                if (!bzData.success) {
                    lastErrorMessage = "BZ calculation failed: " + bzData.errorMessage;
                    SPDLOG_ERROR("{}", lastErrorMessage);
                    return;
                }
                
                SPDLOG_INFO("BZ calculation successful. Facets: {}, Vertices in first facet: {}", 
                           bzData.facets.size(), 
                           bzData.facets.empty() ? 0 : bzData.facets[0].vertices.size());
                
                // 3. 占쏙옙占쏙옙 占쏙옙체 占쏙옙占쏙옙占?(占쏙옙占쏙옙, 占쏙옙占쏙옙, Unit Cell)
                SPDLOG_INFO("Hiding crystal structure...");
                
                if (auto* vtkRenderer = m_parent->vtkRenderer()) {
                    vtkRenderer->setAllAtomGroupsVisible(false);
                    vtkRenderer->setAllBondGroupsVisible(false);
                    
                    if (m_parent->isCellVisible()) {
                        vtkRenderer->setUnitCellVisible(false);
                    }
                }
                
                // ? 4. "All" 키占쏙옙占쏙옙 처占쏙옙 - 占쌘듸옙 占쏙옙占?占쏙옙占쏙옙
                std::string pathStr(pathInput);
                
                if (pathStr == "All" || pathStr == "all" || pathStr == "ALL") {
                    SPDLOG_INFO("'All' keyword detected - determining lattice type automatically");

                    // ? SpecialPointsDatabase::detectLatticeType() 호占쏙옙
                    std::string latticeType = atoms::domain::SpecialPointsDatabase::detectLatticeType(cellInfo.matrix);
                    
                    SPDLOG_INFO("Detected lattice type: {}", latticeType);
                    
                    // SpecialPointsDatabase占쏙옙占쏙옙 占썩본 占쏙옙占?占쏙옙占쏙옙占쏙옙占쏙옙
                    pathStr = atoms::domain::SpecialPointsDatabase::getDefaultPath(latticeType);
                    
                    SPDLOG_INFO("Auto-selected path: '{}'", pathStr);
                }

                // 5. 占쏙옙占쏙옙占쏙옙 BZ Plot 占쏙옙占쏙옙 (占쏙옙占?占쏙옙占?占쏙옙占쏙옙)
                if (auto* vtkRenderer = m_parent->vtkRenderer()) {
                    SPDLOG_INFO("Creating complete BZ Plot with path: '{}'", pathStr);
                    
                    vtkRenderer->createCompleteBZPlot(
                        bzData,
                        cell, 
                        icell, 
                        showVectors,    // 占쏙옙占쏙옙占쏙옙 占쏙옙占쏙옙 표占쏙옙
                        showLabels,     // 특占쏙옙占쏙옙 占쏙옙 표占쏙옙
                        pathStr,        // 占쏙옙占?占쏙옙占?
                        npointsInput    // 占쏙옙占쏙옙 占쏙옙 占쏙옙占쏙옙
                    );
                    
                    SPDLOG_INFO("Complete BZ Plot created successfully");
                    
                    showingBZ = true;
                } 
                
                else {
                    lastErrorMessage = "VTKRenderer not initialized";
                    SPDLOG_ERROR("{}", lastErrorMessage);
                }
                
            } catch (const std::exception& e) {
                lastErrorMessage = std::string("Exception: ") + e.what();
                SPDLOG_ERROR("BZ Plot rendering failed: {}", lastErrorMessage);
            }
            
        } else {
            // ============================================================
            // Crystal 표占쏙옙 占쏙옙占쏙옙 占쏙옙환
            // ============================================================
            SPDLOG_INFO("=== Switching to Crystal mode ===");
            
            // 1. BZ Plot 占쏙옙占쏙옙占?
            if (auto* vtkRenderer = m_parent->vtkRenderer()) {
                SPDLOG_INFO("Hiding BZ Plot...");
                vtkRenderer->setBZPlotVisible(false);
            }
            
            // 2. 占쏙옙占쏙옙 占쏙옙체 占쌕쏙옙 표占쏙옙 (占쏙옙占쏙옙, 占쏙옙占쏙옙, Unit Cell)
            SPDLOG_INFO("Showing crystal structure...");
            
            if (auto* vtkRenderer = m_parent->vtkRenderer()) {
                vtkRenderer->setAllAtomGroupsVisible(true);
                vtkRenderer->setAllBondGroupsVisible(true);
                
                if (m_parent->isCellVisible()) {
                    vtkRenderer->setUnitCellVisible(true);
                }
            }
            
            showingBZ = false;
        }
        
        // 占쏙옙占쏙옙占쏙옙 占쏙옙占쏙옙占쏙옙트
        render::application::GetRenderGateway().RequestRender();
    }
    
    if (sameLineButtons) {
        ImGui::SameLine();
    }
    
    // Clear 占쏙옙튼
    if (ImGui::Button("Clear BZ", buttonSize)) {
        if (auto* vtkRenderer = m_parent->vtkRenderer()) {
            vtkRenderer->clearBZPlot();
            
            // Crystal 占쏙옙占쏙옙 占쏙옙占싣곤옙占쏙옙
            if (showingBZ) {
                if (auto* restoreRenderer = m_parent->vtkRenderer()) {
                    restoreRenderer->setAllAtomGroupsVisible(true);
                    restoreRenderer->setAllBondGroupsVisible(true);
                    
                    if (m_parent->isCellVisible()) {
                        restoreRenderer->setUnitCellVisible(true);
                    }
                }
                showingBZ = false;
            }
            
            lastErrorMessage.clear();
            SPDLOG_INFO("BZ Plot cleared");
            render::application::GetRenderGateway().RequestRender();
        }
    }
    
    // ========================================================================
    // 3. 占쏙옙占쏙옙 표占쏙옙
    // ========================================================================
    
    ImGui::Spacing();
    
    // 占쏙옙占쏙옙 占쏙옙占?표占쏙옙
    if (showingBZ) {
        ImGui::TextColored(ImVec4(0.0f, 0.5f, 1.0f, 1.0f), 
                         "占쏙옙 BZ Plot Mode");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), 
                         "(vectors: %s, labels: %s)", 
                         showVectors ? "ON" : "OFF",
                         showLabels ? "ON" : "OFF");
    } else {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), 
                         "占쏙옙 Crystal Mode");
    }
    
    // 占쏙옙占쏙옙 占쌨쏙옙占쏙옙 표占쏙옙
    if (!lastErrorMessage.empty()) {
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
        ImGui::TextWrapped("Error: %s", lastErrorMessage.c_str());
        ImGui::PopStyleColor();
    }
    
    // ========================================================================
    // 4. 占쌩곤옙 占쏙옙占쏙옙
    // ========================================================================
    
    ImGui::Spacing();
    
    // 占쏙옙占쏙옙 타占쏙옙
    std::string latticeType;
    latticeType = atoms::domain::SpecialPointsDatabase::detectLatticeType(cellInfo.matrix);

    // special points 占쏙옙占쏙옙占쏙옙占쏙옙
    auto special_points = atoms::domain::SpecialPointsDatabase::getSpecialPoints(latticeType);

    // special points 占쏙옙占?
    // atoms::domain::SpecialPointsDatabase::printSpecialPointsTable(special_points);
    
    if (special_points.empty()) {
        ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.0f, 1.0f), "No special points to display");
    }
    
    else {
        ImGui::Text("Total points: %zu", special_points.size());
        ImGui::Separator();
        ImGui::Spacing();
        
        // ImGui 占쏙옙占싱븝옙 占쏙옙占쏙옙
        ImGuiTableFlags tableFlags = 
            ImGuiTableFlags_Borders | 
            ImGuiTableFlags_RowBg | 
            ImGuiTableFlags_SizingFixedFit;
        
        ImGui::BeginTable("SpecialPointsTable", 4, tableFlags);
        // 占쏙옙占?占쏙옙占쏙옙
        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 60.0f);
        ImGui::TableSetupColumn("a", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableSetupColumn("b", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableSetupColumn("c", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableHeadersRow();
        
        // 占쏙옙占쏙옙占쏙옙 占쏙옙
        for (const auto& [label, coords] : special_points) {
            ImGui::TableNextRow();
            
            // Label 占쏙옙
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", label.c_str());
            
            // a 占쏙옙
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%.6f", coords[0]);
            
            // b 占쏙옙
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%.6f", coords[1]);
            
            // c 占쏙옙
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%.6f", coords[2]);
        }
        ImGui::EndTable();
    }
}

} // namespace ui
} // namespace atoms










