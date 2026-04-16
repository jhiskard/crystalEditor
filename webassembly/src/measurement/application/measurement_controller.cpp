#include "../../workspace/legacy/atoms_template_facade.h"

#include "../../app.h"
#include "../../render/application/render_gateway.h"

#include <algorithm>

void AtomsTemplate::EnterMeasurementMode(MeasurementMode mode) {
    if (mode == MeasurementMode::None) {
        ExitMeasurementMode();
        return;
    }
    if (m_MeasurementMode == mode) {
        m_MeasurementPickedAtomIds.clear();
        clearMeasurementPickVisuals();
        ClearSelection();
        render::application::GetRenderGateway().RequestRender();
        return;
    }

    m_MeasurementMode = mode;
    m_MeasurementPickedAtomIds.clear();
    clearMeasurementPickVisuals();
    syncCreatedAtomSelectionVisuals();
    ClearSelection();
    render::application::GetRenderGateway().RequestRender();
}


void AtomsTemplate::ExitMeasurementMode() {
    if (m_MeasurementMode == MeasurementMode::None && m_MeasurementPickedAtomIds.empty()) {
        return;
    }
    m_MeasurementMode = MeasurementMode::None;
    m_MeasurementPickedAtomIds.clear();
    clearMeasurementPickVisuals();
    syncCreatedAtomSelectionVisuals();
    ClearSelection();
    render::application::GetRenderGateway().RequestRender();
}


void AtomsTemplate::HandleMeasurementClickByPicker(vtkActor* actor, double pickPos[3]) {
    if (!IsMeasurementModeActive()) {
        return;
    }

    const bool centerMode = isCenterMeasurementMode();
    const size_t targetPickCount = measurementTargetPickCount();
    if (!centerMode && targetPickCount == 0) {
        return;
    }

    uint32_t atomId = 0;
    int32_t structureId = -1;
    std::array<double, 3> atomPosition {};
    if (!resolvePickedAtom(actor, pickPos, atomId, structureId, atomPosition)) {
        HandleMeasurementEmptyClick();
        return;
    }
    (void)atomPosition;
    if (structureId < 0) {
        return;
    }

    if (!centerMode && m_MeasurementPickedAtomIds.size() >= targetPickCount) {
        m_MeasurementPickedAtomIds.clear();
    }

    if (m_MeasurementPickedAtomIds.empty()) {
        m_MeasurementPickedAtomIds.push_back(atomId);
        syncMeasurementPickVisuals();
        render::application::GetRenderGateway().RequestRender();
        return;
    }

    if (std::find(
            m_MeasurementPickedAtomIds.begin(),
            m_MeasurementPickedAtomIds.end(),
            atomId) != m_MeasurementPickedAtomIds.end()) {
        syncMeasurementPickVisuals();
        render::application::GetRenderGateway().RequestRender();
        return;
    }

    const atoms::domain::AtomInfo* firstAtom = findAtomById(m_MeasurementPickedAtomIds.front());
    if (!firstAtom || firstAtom->structureId != structureId) {
        m_MeasurementPickedAtomIds.clear();
        m_MeasurementPickedAtomIds.push_back(atomId);
        syncMeasurementPickVisuals();
        render::application::GetRenderGateway().RequestRender();
        return;
    }

    if (!centerMode && m_MeasurementPickedAtomIds.size() >= targetPickCount) {
        m_MeasurementPickedAtomIds.clear();
        m_MeasurementPickedAtomIds.push_back(atomId);
        syncMeasurementPickVisuals();
        render::application::GetRenderGateway().RequestRender();
        return;
    }

    m_MeasurementPickedAtomIds.push_back(atomId);
    syncMeasurementPickVisuals();
    if (!centerMode && m_MeasurementPickedAtomIds.size() == targetPickCount) {
        createMeasurementFromPickedAtoms();
    }
    render::application::GetRenderGateway().RequestRender();
}


void AtomsTemplate::RenderMeasurementModeOverlay() {
    if (!IsMeasurementModeActive()) {
        return;
    }

    const char* modeLabel = "Measurement Mode";
    const bool centerMode = isCenterMeasurementMode();
    if (m_MeasurementMode == MeasurementMode::Distance) {
        modeLabel = "Distance Mode";
    } else if (m_MeasurementMode == MeasurementMode::Angle) {
        modeLabel = "Angle Mode";
    } else if (m_MeasurementMode == MeasurementMode::Dihedral) {
        modeLabel = "Dihedral Mode";
    } else if (m_MeasurementMode == MeasurementMode::GeometricCenter) {
        modeLabel = "Geometric Center Mode";
    } else if (m_MeasurementMode == MeasurementMode::CenterOfMass) {
        modeLabel = "Center of Mass Mode";
    }

    const ImVec2 contentMin = ImGui::GetWindowContentRegionMin();
    const float overlayX = contentMin.x + 12.0f * App::UiScale();
    const float toolbarTopPadding = 5.0f * static_cast<float>(App::DevicePixelRatio());
    const float toolbarReservedHeight = 56.0f * App::UiScale();
    const float overlayY = contentMin.y + toolbarTopPadding + toolbarReservedHeight;
    ImGui::SetCursorPos(ImVec2(overlayX, overlayY));
    const ImVec2 overlayPadding(12.0f * App::UiScale(), 10.0f * App::UiScale());
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, overlayPadding);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 0.35f));
    const ImGuiChildFlags overlayChildFlags =
        ImGuiChildFlags_AutoResizeX |
        ImGuiChildFlags_AutoResizeY |
        ImGuiChildFlags_AlwaysUseWindowPadding;
    if (ImGui::BeginChild(
            "##MeasurementModeOverlay",
            ImVec2(0.0f, 0.0f),
            overlayChildFlags,
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(modeLabel);
        ImGui::SameLine();
        if (ImGui::Button("Exit")) {
            ExitMeasurementMode();
        }

        if (centerMode) {
            const size_t pickedCount = m_MeasurementPickedAtomIds.size();
            const bool canApply = pickedCount >= 2;

            ImGui::Spacing();
            ImGui::Text("Selected: %zu", pickedCount);

            bool applyTriggered = false;
            ImGui::BeginDisabled(!canApply);
            if (ImGui::Button("Apply")) {
                applyTriggered = true;
            }
            ImGui::EndDisabled();
            ImGui::SameLine();
            if (ImGui::Button("Clear")) {
                m_MeasurementPickedAtomIds.clear();
                clearMeasurementPickVisuals();
                render::application::GetRenderGateway().RequestRender();
            }

            const ImGuiIO& io = ImGui::GetIO();
            const bool typing = io.WantTextInput || ImGui::IsAnyItemActive();
            if (!typing && canApply &&
                (ImGui::IsKeyPressed(ImGuiKey_Enter, false) ||
                 ImGui::IsKeyPressed(ImGuiKey_KeypadEnter, false))) {
                applyTriggered = true;
            }

            if (applyTriggered) {
                commitCenterMeasurementFromPickedAtoms();
            }
        }

        bool* stylePanelExpanded = nullptr;
        switch (m_MeasurementMode) {
        case MeasurementMode::Distance:
            stylePanelExpanded = &m_DistanceStylePanelExpanded;
            break;
        case MeasurementMode::Angle:
            stylePanelExpanded = &m_AngleStylePanelExpanded;
            break;
        case MeasurementMode::Dihedral:
            stylePanelExpanded = &m_DihedralStylePanelExpanded;
            break;
        case MeasurementMode::GeometricCenter:
            stylePanelExpanded = &m_GeometricCenterStylePanelExpanded;
            break;
        case MeasurementMode::CenterOfMass:
            stylePanelExpanded = &m_CenterOfMassStylePanelExpanded;
            break;
        default:
            break;
        }
        if (stylePanelExpanded) {
            ImGui::Spacing();
            if (ImGui::Button(*stylePanelExpanded ? "Preference (Collapse)" : "Preference (Expand)")) {
                *stylePanelExpanded = !(*stylePanelExpanded);
            }
            if (*stylePanelExpanded) {
                renderMeasurementStyleOptionsOverlay();
            }
        }
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);
}



