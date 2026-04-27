#include "bz_plot_ui.h"
#include "../../../workspace/runtime/legacy_atoms_runtime.h"
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

    // Path �Է�
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

    // Npoints �Է�
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

    // ?? ��� ���� ���� ����
    bool prevShowVectors = m_showVectors;
    bool prevShowLabels  = m_showLabels;

    // ?? ���� ǥ�� ���
    ImGui::Checkbox("Show reciprocal vectors", &m_showVectors);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Toggle visibility of reciprocal lattice vectors.");
    }

    // ?? �� ǥ�� ���
    ImGui::Checkbox("Show special point labels", &m_showLabels);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Toggle labels for special k-points.");
    }

    // ?? �� �� �ϳ��� ���� �ٲ������ Ȯ��
    bool vectorsChanged = (prevShowVectors != m_showVectors);
    bool labelsChanged  = (prevShowLabels  != m_showLabels);
    bool optionsChanged = vectorsChanged || labelsChanged;

    // �̹� BZ Plot ����� ���� ��� ������Ʈ
    if (optionsChanged && m_showingBZ && m_parent) {
        m_lastErrorMessage.clear();

        std::string pathStr(m_pathInput);
        int npoints = m_npointsInput;

        // ?? �߿�: ���� �ɼ�(m_showVectors / m_showLabels)�� �״�� �Ѱܼ�
        // ��ü BZ Plot�� �ٽ� �����ϵ��� �Ѵ�.
        bool success = m_parent->EnterBZPlotMode(
            pathStr,
            npoints,
            m_showVectors,   // �� ���� ǥ�� ����
            m_showLabels,    // �� �� ǥ�� ���� (���Ⱑ �ٽ�)
            m_lastErrorMessage
        );

        // �ٽ� �׸��� ���� ���ο� ���� �α׸� ��������� ���
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

    // Show BZ Plot / Show Crystal ��� ��ư
    if (ImGui::Button(buttonText, buttonSize)) {
        m_lastErrorMessage.clear();

        if (!m_showingBZ) {
            // BZ Plot ��� ����
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
            // Crystal ���� ����
            if (m_parent) {
                m_parent->ExitBZPlotMode();
            }
            m_showingBZ = false;
        }
    }

    if (sameLineButtons) {
        ImGui::SameLine();
    }

    // Clear BZ ��ư
    if (ImGui::Button("Clear BZ", buttonSize)) {
        m_lastErrorMessage.clear();
        if (m_parent) {
            // BZ Plot�� �����ϰ� crystal ���� ����
            m_parent->ExitBZPlotMode();
        }
        m_showingBZ = false;
    }
}

void BZPlotUI::renderStatus() {
    ImGui::Spacing();

    if (m_showingBZ) {
        ImGui::TextColored(ImVec4(0.0f, 0.5f, 1.0f, 1.0f),
                           "�� BZ Plot Mode");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                           "(vectors: %s, labels: %s)",
                           m_showVectors ? "ON" : "OFF",
                           m_showLabels ? "ON" : "OFF");
    } else {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f),
                           "�� Crystal Mode");
    }

    if (!m_lastErrorMessage.empty()) {
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
        ImGui::TextWrapped("Error: %s", m_lastErrorMessage.c_str());
        ImGui::PopStyleColor();
    }
}

void BZPlotUI::renderSpecialPointsTable() {
    // ���� Ÿ�� ���� �� Ư���� ������ ��ȸ
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
// ���� static ���� �������� ����� �̵�
char  m_pathInput[128] = "All";
int   m_npointsInput   = 50;
bool  m_showVectors    = true;
bool  m_showLabels     = true;
bool  m_showingBZ      = false;
*/
void BZPlotUI::renderBZplot() {
    // �Է� �ʵ� ���� (static���� ����)
    static char pathInput[128] = "All";  // ? �⺻�� "All"�� ����
    static int npointsInput = 50;
    static bool showVectors = true;
    static bool showLabels = true;
    static bool showingBZ = false;  // BZ ǥ�� ����
    static std::string lastErrorMessage;
    
    // ========================================================================
    // 1. �Է� UI
    // ========================================================================
    
    ImGui::Text("Bandpath Configuration:");
    ImGui::Spacing();
    
    // Path �Է�
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
    
    // Npoints �Է�
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
    
    // �ɼ� üũ�ڽ�
    ImGui::Checkbox("Show reciprocal vectors", &showVectors);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Display the reciprocal lattice vectors (b1, b2, b3)");
    }
    
    ImGui::Checkbox("Show special point labels", &showLabels);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Display labels at high-symmetry points (��, X, M, etc.)");
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // ========================================================================
    // 2. ��� ��ư (Show BZ Plot �� Show Crystal)
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
            // BZ Plot ǥ�� ���� ��ȯ
            // ============================================================
            SPDLOG_INFO("=== Switching to BZ Plot mode ===");
            SPDLOG_INFO("Path: '{}', npoints: {}, vectors: {}, labels: {}", 
                       pathInput, npointsInput, showVectors, showLabels);
            
            lastErrorMessage.clear();
            
            try {
                // 1. ������ ��� �غ�
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
                
                // 2. BZ vertices ���
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
                
                // 3. ���� ��ü ����� (����, ����, Unit Cell)
                SPDLOG_INFO("Hiding crystal structure...");
                
                if (auto* vtkRenderer = m_parent->vtkRenderer()) {
                    vtkRenderer->setAllAtomGroupsVisible(false);
                    vtkRenderer->setAllBondGroupsVisible(false);
                    
                    if (m_parent->isCellVisible()) {
                        vtkRenderer->setUnitCellVisible(false);
                    }
                }
                
                // ? 4. "All" Ű���� ó�� - �ڵ� ��� ����
                std::string pathStr(pathInput);
                
                if (pathStr == "All" || pathStr == "all" || pathStr == "ALL") {
                    SPDLOG_INFO("'All' keyword detected - determining lattice type automatically");

                    // ? SpecialPointsDatabase::detectLatticeType() ȣ��
                    std::string latticeType = atoms::domain::SpecialPointsDatabase::detectLatticeType(cellInfo.matrix);
                    
                    SPDLOG_INFO("Detected lattice type: {}", latticeType);
                    
                    // SpecialPointsDatabase���� �⺻ ��� ��������
                    pathStr = atoms::domain::SpecialPointsDatabase::getDefaultPath(latticeType);
                    
                    SPDLOG_INFO("Auto-selected path: '{}'", pathStr);
                }

                // 5. ������ BZ Plot ���� (��� ��� ����)
                if (auto* vtkRenderer = m_parent->vtkRenderer()) {
                    SPDLOG_INFO("Creating complete BZ Plot with path: '{}'", pathStr);
                    
                    vtkRenderer->createCompleteBZPlot(
                        bzData,
                        cell, 
                        icell, 
                        showVectors,    // ������ ���� ǥ��
                        showLabels,     // Ư���� �� ǥ��
                        pathStr,        // ��� ���
                        npointsInput    // ���� �� ����
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
            // Crystal ǥ�� ���� ��ȯ
            // ============================================================
            SPDLOG_INFO("=== Switching to Crystal mode ===");
            
            // 1. BZ Plot �����
            if (auto* vtkRenderer = m_parent->vtkRenderer()) {
                SPDLOG_INFO("Hiding BZ Plot...");
                vtkRenderer->setBZPlotVisible(false);
            }
            
            // 2. ���� ��ü �ٽ� ǥ�� (����, ����, Unit Cell)
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
        
        // ������ ������Ʈ
        render::application::GetRenderGateway().RequestRender();
    }
    
    if (sameLineButtons) {
        ImGui::SameLine();
    }
    
    // Clear ��ư
    if (ImGui::Button("Clear BZ", buttonSize)) {
        if (auto* vtkRenderer = m_parent->vtkRenderer()) {
            vtkRenderer->clearBZPlot();
            
            // Crystal ���� ���ư���
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
    // 3. ���� ǥ��
    // ========================================================================
    
    ImGui::Spacing();
    
    // ���� ��� ǥ��
    if (showingBZ) {
        ImGui::TextColored(ImVec4(0.0f, 0.5f, 1.0f, 1.0f), 
                         "�� BZ Plot Mode");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), 
                         "(vectors: %s, labels: %s)", 
                         showVectors ? "ON" : "OFF",
                         showLabels ? "ON" : "OFF");
    } else {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), 
                         "�� Crystal Mode");
    }
    
    // ���� �޽��� ǥ��
    if (!lastErrorMessage.empty()) {
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
        ImGui::TextWrapped("Error: %s", lastErrorMessage.c_str());
        ImGui::PopStyleColor();
    }
    
    // ========================================================================
    // 4. �߰� ����
    // ========================================================================
    
    ImGui::Spacing();
    
    // ���� Ÿ��
    std::string latticeType;
    latticeType = atoms::domain::SpecialPointsDatabase::detectLatticeType(cellInfo.matrix);

    // special points ��������
    auto special_points = atoms::domain::SpecialPointsDatabase::getSpecialPoints(latticeType);

    // special points ���
    // atoms::domain::SpecialPointsDatabase::printSpecialPointsTable(special_points);
    
    if (special_points.empty()) {
        ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.0f, 1.0f), "No special points to display");
    }
    
    else {
        ImGui::Text("Total points: %zu", special_points.size());
        ImGui::Separator();
        ImGui::Spacing();
        
        // ImGui ���̺� ����
        ImGuiTableFlags tableFlags = 
            ImGuiTableFlags_Borders | 
            ImGuiTableFlags_RowBg | 
            ImGuiTableFlags_SizingFixedFit;
        
        ImGui::BeginTable("SpecialPointsTable", 4, tableFlags);
        // ��� ����
        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 60.0f);
        ImGui::TableSetupColumn("a", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableSetupColumn("b", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableSetupColumn("c", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableHeadersRow();
        
        // ������ ��
        for (const auto& [label, coords] : special_points) {
            ImGui::TableNextRow();
            
            // Label ��
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", label.c_str());
            
            // a ��
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%.6f", coords[0]);
            
            // b ��
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%.6f", coords[1]);
            
            // c ��
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%.6f", coords[2]);
        }
        ImGui::EndTable();
    }
}

} // namespace ui
} // namespace atoms










