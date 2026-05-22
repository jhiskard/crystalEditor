#include "../../../app.h"

#include "shell/presentation/font/font_scale_controller.h"
#include "shell/runtime/workbench_runtime.h"

#include <imgui.h>

#include <emscripten/emscripten.h>

namespace {

float ToUiScale(App::FontSizePreset preset) {
    switch (preset) {
    case App::FontSizePreset::Small:
        return 1.0f;
    case App::FontSizePreset::Medium:
        return 1.2f;
    case App::FontSizePreset::Large:
        return 1.5f;
    }
    return 1.0f;
}

App::FontSizePreset FontSizePresetFromInt(int value) {
    switch (value) {
    case 1:
        return App::FontSizePreset::Medium;
    case 2:
        return App::FontSizePreset::Large;
    default:
        return App::FontSizePreset::Large;
    }
}

}  // namespace

void App::SetFontSizePreset(FontSizePreset preset) {
    GetWorkbenchRuntime().AppController().setFontSizePreset(preset);
}

App::FontSizePreset App::GetFontSizePreset() {
    return GetWorkbenchRuntime().AppController().getFontSizePreset();
}

float App::UiScale() {
    return GetWorkbenchRuntime().AppController().m_kUiScale;
}

void App::setFontSizePreset(FontSizePreset preset) {
    m_FontSizePreset = preset;
    m_kUiScale = ToUiScale(preset);
    applyFontScale();

    EM_ASM({
        const fontSizeKey = UTF8ToString($0);
        const fontSizePreset = $1;
        localStorage.setItem(fontSizeKey, fontSizePreset);
    }, IMGUI_FONT_SIZE_KEY, static_cast<int>(m_FontSizePreset));
}

void App::applyFontScale() {
    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = m_kUiScale;
}

void App::SetInitFontSize() {
    const int fontSizePreset = EM_ASM_INT({
        const fontSizeKey = UTF8ToString($0);
        const fontSize = localStorage.getItem(fontSizeKey);
        if (fontSize) {
            const parsed = parseInt(fontSize, 10);
            return Number.isNaN(parsed) ? 0 : parsed;
        }
        return 0;
    }, IMGUI_FONT_SIZE_KEY);

    setFontSizePreset(FontSizePresetFromInt(fontSizePreset));
}
