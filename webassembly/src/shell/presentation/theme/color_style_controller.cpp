#include "../../../app.h"

#include "shell/presentation/theme/color_style_controller.h"
#include "shell/runtime/workbench_runtime.h"

#include <imgui.h>

#include <emscripten/emscripten.h>

void App::SetColorStyle(ColorStyle style) {
    GetWorkbenchRuntime().AppController().setColorStyle(style);
}

ColorStyle App::GetColorStyle() {
    return GetWorkbenchRuntime().AppController().getColorStyle();
}

void App::setColorStyle(ColorStyle style) {
    m_ColorStyle = style;

    switch (style) {
    case ColorStyle::Dark:
        ImGui::StyleColorsDark();
        break;
    case ColorStyle::Light:
        ImGui::StyleColorsLight();
        break;
    case ColorStyle::Classic:
        ImGui::StyleColorsClassic();
        break;
    }

    EM_ASM({
        const styleKey = UTF8ToString($0);
        const style = $1;
        localStorage.setItem(styleKey, style);
    }, IMGUI_STYLE_KEY, static_cast<int>(m_ColorStyle));
}

void App::SetDetailedStyle() {
    ImGuiStyle& style = ImGui::GetStyle();

    style.WindowPadding = ImVec2(6.0f, 6.0f);
    style.FramePadding = ImVec2(5.0f, 3.0f);
    style.ItemSpacing = ImVec2(6.0f, 4.0f);
    style.IndentSpacing = 8.0f;
    style.ScrollbarSize = 9.0f;
    style.GrabMinSize = 8.0f;

    style.FrameBorderSize = 0.5f;
    style.TabBorderSize = 1.0f;

    style.WindowRounding = 0.0f;
    style.FrameRounding = 4.0f;
    style.TabRounding = 4.0f;
    style.ScrollbarRounding = 6.0f;
    style.GrabRounding = 6.0f;

    style.CellPadding = ImVec2(6.0f, 3.0f);

    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    style.Colors[ImGuiCol_Button].w = 0.2f;
}

void App::SetInitColorStyle() {
    const int colorStyle = EM_ASM_INT({
        const styleKey = UTF8ToString($0);
        const style = localStorage.getItem(styleKey);
        if (style) {
            return style;
        }
        return 0;
    }, IMGUI_STYLE_KEY);

    const ColorStyle initColorStyle = static_cast<ColorStyle>(colorStyle);
    setColorStyle(initColorStyle);
}
