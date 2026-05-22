#pragma once

#include <imgui.h>

namespace shell::presentation {

struct ResetWindowLayout {
    ImVec2 viewerPos;
    ImVec2 viewerSize;
    ImVec2 modelTreePos;
    ImVec2 advancedViewPos;
    ImVec2 crystalBuilderPos;
    ImVec2 crystalEditorPos;
    ImVec2 panelSize;
};

ResetWindowLayout BuildResetWindowLayout(const ImGuiViewport* viewport);

}  // namespace shell::presentation
