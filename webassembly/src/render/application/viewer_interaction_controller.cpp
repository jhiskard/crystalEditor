#include "../presentation/viewer_window.h"

#include "../../app.h"
#include "../../mesh_detail.h"
#include "../../atoms/atoms_template.h"
#include "../../shell/runtime/workbench_runtime.h"

#include <vtkCommand.h>

#include <imgui.h>
#include <algorithm>
#include <chrono>
#include <cmath>

namespace {
constexpr auto kWheelInteractionLodHoldDuration = std::chrono::milliseconds(150);

bool isWheelInteractionLodHoldActive(const std::chrono::steady_clock::time_point& lastWheelTime,
    const std::chrono::steady_clock::time_point& now) {
    if (lastWheelTime == std::chrono::steady_clock::time_point{}) {
        return false;
    }
    return (now - lastWheelTime) <= kWheelInteractionLodHoldDuration;
}
} // namespace

void VtkViewer::processEvents() {
    ImGuiIO& io = ImGui::GetIO();
    const auto now = std::chrono::steady_clock::now();
    const bool anyMouseDown = io.MouseDown[ImGuiMouseButton_Left] ||
        io.MouseDown[ImGuiMouseButton_Right] ||
        io.MouseDown[ImGuiMouseButton_Middle];
    const bool hasWheelInput = io.MouseWheel != 0.0f;
    if (hasWheelInput) {
        m_LastWheelInteractionTime = now;
    }
    const bool wheelLodHoldActive = isWheelInteractionLodHoldActive(m_LastWheelInteractionTime, now);
    const bool shouldKeepInteractionLod = anyMouseDown || wheelLodHoldActive;
    
    static bool isDraggingWindow = false;
    static ImVec2 dragStartPos;
    
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 viewportPos = ImGui::GetCursorStartPos();
    auto toViewerLocalClamped = [&](const ImVec2& mousePos) -> ImVec2 {
        const float relX = mousePos.x - windowPos.x - viewportPos.x;
        const float relY = mousePos.y - windowPos.y - viewportPos.y;
        const float maxX = static_cast<float>(std::max(0, m_Width));
        const float maxY = static_cast<float>(std::max(0, m_Height));
        return ImVec2(
            std::clamp(relX, 0.0f, maxX),
            std::clamp(relY, 0.0f, maxY));
    };
    
    // Dragging check
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        dragStartPos = io.MousePos;
        
        if (io.MousePos.y >= windowPos.y && io.MousePos.y <= windowPos.y + App::TitleBarHeight() &&
            io.MousePos.x >= windowPos.x && io.MousePos.x <= windowPos.x + ImGui::GetWindowWidth()) {
            isDraggingWindow = true;
        }
    }
    
    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
        isDraggingWindow = false;
    }
    
    if (isDraggingWindow) {
        if (io.MouseReleased[ImGuiMouseButton_Left] && m_DragSelection.tracking) {
            resetDragSelection();
            RequestRender();
        }
        if (m_InteractionLodActive && !shouldKeepInteractionLod) {
            EndInteractionLod();
            RequestRender();
        }
        return;
    }
    
    if (!ImGui::IsWindowFocused() && !ImGui::IsWindowHovered()) {
        if (io.MouseReleased[ImGuiMouseButton_Left] && m_DragSelection.tracking) {
            resetDragSelection();
            RequestRender();
        }
        if (m_InteractionLodActive && !shouldKeepInteractionLod) {
            EndInteractionLod();
            RequestRender();
        }
        return;
    }

    io.ConfigWindowsMoveFromTitleBarOnly = true;

    int xPos = static_cast<int>(io.MousePos.x - windowPos.x - viewportPos.x);
    int yPos = static_cast<int>(io.MousePos.y - windowPos.y - viewportPos.y);
    int ctrl = static_cast<int>(io.KeyCtrl);
    int shift = static_cast<int>(io.KeyShift);
    bool dclick = io.MouseDoubleClicked[0] || io.MouseDoubleClicked[1] || io.MouseDoubleClicked[2];

    m_Interactor->SetEventInformationFlipY(xPos, yPos, ctrl, shift, dclick);

    static int32_t clickXPos = 0;
    static int32_t clickYPos = 0;
    static bool hasLastPos = false;
    static int32_t lastXPos = 0;
    static int32_t lastYPos = 0;
    static bool leftPressForwardedToInteractor = false;
    static bool pendingMeasurementPick = false;
    bool requestRender = false;
    AtomsTemplate& atomsTemplate = GetWorkbenchRuntime().AtomsTemplateFacade();
    const bool measurementModeActive = atomsTemplate.IsMeasurementModeActive();
    const bool measurementDragSelectionEnabled =
        measurementModeActive && atomsTemplate.IsMeasurementDragSelectionEnabled();
    const bool selectionModifierDown = io.KeySuper || io.KeyCtrl;
    const bool canStartModifierDragSelection =
        selectionModifierDown && (!measurementModeActive || measurementDragSelectionEnabled);
    const bool windowHovered = ImGui::IsWindowHovered();

    if (windowHovered) {
        float relX = io.MousePos.x - windowPos.x - viewportPos.x;
        float relY = io.MousePos.y - windowPos.y - viewportPos.y;
        
        int pickX = static_cast<int>(relX);
        int pickY = static_cast<int>(m_Height - relY);
        const ImVec2 clampedLocalPos = toViewerLocalClamped(io.MousePos);

        if ((io.MouseClicked[ImGuiMouseButton_Left] ||
            io.MouseClicked[ImGuiMouseButton_Right] ||
            io.MouseClicked[ImGuiMouseButton_Middle]) &&
            !m_InteractionLodActive) {
            BeginInteractionLod();
        }
        
        if (io.MouseClicked[ImGuiMouseButton_Left]) {
            clickXPos = io.MousePos.x;
            clickYPos = io.MousePos.y;

            leftPressForwardedToInteractor = false;
            pendingMeasurementPick = false;
            const bool sceneInputBlockedByUi = io.WantCaptureMouse && ImGui::IsAnyItemHovered();
            if (sceneInputBlockedByUi) {
                requestRender = true;
                if (m_DragSelection.tracking) {
                    resetDragSelection();
                }
            } else if (canStartModifierDragSelection) {
                m_DragSelection.tracking = true;
                m_DragSelection.active = false;
                m_DragSelection.measurementModeAtStart = measurementModeActive;
                m_DragSelection.sameElementSelectAtStart = io.MouseDoubleClicked[ImGuiMouseButton_Left];
                m_DragSelection.startPos = clampedLocalPos;
                m_DragSelection.currentPos = clampedLocalPos;
                requestRender = true;
            } else if (measurementModeActive) {
                m_Interactor->InvokeEvent(vtkCommand::LeftButtonPressEvent, nullptr);
                leftPressForwardedToInteractor = true;
                pendingMeasurementPick = true;
                requestRender = true;
            } else {
                m_Interactor->InvokeEvent(vtkCommand::LeftButtonPressEvent, nullptr);
                leftPressForwardedToInteractor = true;
                requestRender = true;
            }
        }
        else if (io.MouseClicked[ImGuiMouseButton_Right]) {
            m_Interactor->InvokeEvent(vtkCommand::RightButtonPressEvent, nullptr);
            ImGui::SetWindowFocus();
            requestRender = true;
        }
        else if (io.MouseClicked[ImGuiMouseButton_Middle]) {
            m_Interactor->InvokeEvent(vtkCommand::MiddleButtonPressEvent, nullptr);
            requestRender = true;
        }
        else if (io.MouseWheel > 0) {
            if (!m_InteractionLodActive) {
                BeginInteractionLod();
            }
            m_LastWheelInteractionTime = now;
            m_Interactor->InvokeEvent(vtkCommand::MouseWheelForwardEvent, nullptr);
            requestRender = true;
        }
        else if (io.MouseWheel < 0) {
            if (!m_InteractionLodActive) {
                BeginInteractionLod();
            }
            m_LastWheelInteractionTime = now;
            m_Interactor->InvokeEvent(vtkCommand::MouseWheelBackwardEvent, nullptr);
            requestRender = true;
        }
    }

    if (m_DragSelection.tracking && io.MouseDown[ImGuiMouseButton_Left]) {
        m_DragSelection.currentPos = toViewerLocalClamped(io.MousePos);
        const float movedDist =
            std::fabs(m_DragSelection.currentPos.x - m_DragSelection.startPos.x) +
            std::fabs(m_DragSelection.currentPos.y - m_DragSelection.startPos.y);
        if (!m_DragSelection.active && movedDist > 4.0f) {
            m_DragSelection.active = true;
        }
        if (m_DragSelection.active) {
            requestRender = true;
        }
    }

    bool releasedAnyButton = false;
    if (io.MouseReleased[ImGuiMouseButton_Left]) {
        int32_t movedDist = abs(static_cast<int32_t>(io.MousePos.x) - clickXPos) +
            abs(static_cast<int32_t>(io.MousePos.y) - clickYPos);
        if (m_DragSelection.tracking) {
            m_DragSelection.currentPos = toViewerLocalClamped(io.MousePos);
            if (m_DragSelection.active) {
                const int dragX0 = static_cast<int>(std::lround(m_DragSelection.startPos.x));
                const int dragY0 = static_cast<int>(std::lround(m_DragSelection.startPos.y));
                const int dragX1 = static_cast<int>(std::lround(m_DragSelection.currentPos.x));
                const int dragY1 = static_cast<int>(std::lround(m_DragSelection.currentPos.y));
                atomsTemplate.HandleDragSelectionInScreenRect(
                    dragX0,
                    dragY0,
                    dragX1,
                    dragY1,
                    m_Renderer,
                    m_Height,
                    true);
            } else {
                float relX = io.MousePos.x - windowPos.x - viewportPos.x;
                float relY = io.MousePos.y - windowPos.y - viewportPos.y;
                int pickX = static_cast<int>(relX);
                int pickY = static_cast<int>(m_Height - relY);
                if (m_DragSelection.measurementModeAtStart) {
                    if (m_Picker->Pick(pickX, pickY, 0, m_Renderer)) {
                        vtkActor* pickedActor = m_Picker->GetActor();
                        double* pickPos = m_Picker->GetPickPosition();
                        atomsTemplate.HandleMeasurementClickByPicker(pickedActor, pickPos);
                    } else {
                        atomsTemplate.HandleMeasurementEmptyClick();
                    }
                } else {
                    if (m_Picker->Pick(pickX, pickY, 0, m_Renderer)) {
                        vtkActor* pickedActor = m_Picker->GetActor();
                        double* pickPos = m_Picker->GetPickPosition();
                        if (m_DragSelection.sameElementSelectAtStart) {
                            atomsTemplate.SelectSameElementAtomsByPicker(pickedActor, pickPos);
                        } else {
                            atomsTemplate.SelectAtomByPicker(pickedActor, pickPos);
                        }
                    } else {
                        atomsTemplate.ClearCreatedAtomSelection();
                    }
                }
            }
            resetDragSelection();
            pendingMeasurementPick = false;
            leftPressForwardedToInteractor = false;
            requestRender = true;
            releasedAnyButton = true;
        } else if (leftPressForwardedToInteractor) {
            if (movedDist > 4) {
                setCameraDirection(CameraDirection::NOT_ALIGNED);
            }
            m_Interactor->InvokeEvent(vtkCommand::LeftButtonReleaseEvent, nullptr);
            requestRender = true;
            releasedAnyButton = true;

            if (!measurementModeActive && movedDist <= 4) {
                float relX = io.MousePos.x - windowPos.x - viewportPos.x;
                float relY = io.MousePos.y - windowPos.y - viewportPos.y;
                int pickX = static_cast<int>(relX);
                int pickY = static_cast<int>(m_Height - relY);
                if (!m_Picker->Pick(pickX, pickY, 0, m_Renderer)) {
                    atomsTemplate.ClearCreatedAtomSelection();
                }
            }
        }

        if (pendingMeasurementPick && movedDist <= 4) {
            float relX = io.MousePos.x - windowPos.x - viewportPos.x;
            float relY = io.MousePos.y - windowPos.y - viewportPos.y;
            int pickX = static_cast<int>(relX);
            int pickY = static_cast<int>(m_Height - relY);
            if (m_Picker->Pick(pickX, pickY, 0, m_Renderer)) {
                vtkActor* pickedActor = m_Picker->GetActor();
                double* pickPos = m_Picker->GetPickPosition();
                atomsTemplate.HandleMeasurementClickByPicker(pickedActor, pickPos);
            } else {
                atomsTemplate.HandleMeasurementEmptyClick();
            }
            requestRender = true;
        }

        if (!m_DragSelection.tracking) {
            pendingMeasurementPick = false;
            leftPressForwardedToInteractor = false;
        }
    }
    if (io.MouseReleased[ImGuiMouseButton_Right]) {
        m_Interactor->InvokeEvent(vtkCommand::RightButtonReleaseEvent, nullptr);
        requestRender = true;
        releasedAnyButton = true;
    }
    if (io.MouseReleased[ImGuiMouseButton_Middle]) {
        m_Interactor->InvokeEvent(vtkCommand::MiddleButtonReleaseEvent, nullptr);
        requestRender = true;
        releasedAnyButton = true;
    }

    bool isDragging = (io.MouseDown[ImGuiMouseButton_Left] && leftPressForwardedToInteractor) ||
        (io.MouseDown[ImGuiMouseButton_Left] && m_DragSelection.tracking) ||
        io.MouseDown[ImGuiMouseButton_Right] ||
        io.MouseDown[ImGuiMouseButton_Middle];
    if (releasedAnyButton && !isDragging && m_InteractionLodActive && !wheelLodHoldActive) {
        EndInteractionLod();
        requestRender = true;
    }
    bool moved = !hasLastPos || lastXPos != xPos || lastYPos != yPos;
    if ((moved || isDragging) && !m_DragSelection.tracking) {
        m_Interactor->InvokeEvent(vtkCommand::MouseMoveEvent, nullptr);
        requestRender = true;
    }
    lastXPos = xPos;
    lastYPos = yPos;
    hasLastPos = true;

    if (!isDragging && !wheelLodHoldActive && m_InteractionLodActive) {
        EndInteractionLod();
        requestRender = true;
    }

    const bool isTypingInTextField = io.WantTextInput || ImGui::IsAnyItemActive();
    bool rotatedByArrowKey = false;
    if (!isTypingInTextField) {
        const float stepDeg = GetArrowRotateStepDeg();
        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow, true)) {
            rotatedByArrowKey = RotateCameraByKeyboard(0.0f, stepDeg) || rotatedByArrowKey;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_DownArrow, true)) {
            rotatedByArrowKey = RotateCameraByKeyboard(0.0f, -stepDeg) || rotatedByArrowKey;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow, true)) {
            rotatedByArrowKey = RotateCameraByKeyboard(-stepDeg, 0.0f) || rotatedByArrowKey;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_RightArrow, true)) {
            rotatedByArrowKey = RotateCameraByKeyboard(stepDeg, 0.0f) || rotatedByArrowKey;
        }
    }
    if (rotatedByArrowKey) {
        requestRender = true;
    }

    if (requestRender) {
        RequestRender();
    }
}

