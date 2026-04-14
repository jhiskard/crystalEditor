#include "../ui/charge_density_ui.h"

#include "../atoms_template.h"
#include "../infrastructure/charge_density_renderer.h"
#include "../../config/log_config.h"
#include "../../mesh/application/mesh_command_service.h"
#include "../../mesh/presentation/mesh_detail_panel.h"

#include <algorithm>
#include <cctype>
#include <cmath>

#include <imgui.h>

namespace atoms {
namespace ui {
namespace {
std::string toLowerCopyForIsosurfacePanel(const std::string& input) {
    std::string output = input;
    std::transform(output.begin(), output.end(), output.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return output;
}
}  // namespace

void ChargeDensityUI::renderIsosurfaceSection(const std::vector<GridMeshEntry>& gridMeshes) {
    if (ImGui::TreeNodeEx("Isosurface", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (!m_fileLoaded) {
            if (hasGridDataEntries()) {
                ImGui::TextDisabled("Grid data available. Select grid data above.");
            } else if (gridMeshes.empty()) {
                ImGui::TextDisabled("No grid data loaded");
            } else {
                bool structureVisible = true;
                if (m_atomsTemplate) {
                    int32_t structureId = m_atomsTemplate->GetCurrentStructureId();
                    structureVisible = m_atomsTemplate->IsStructureVisible(structureId);
                }

                ImGui::BeginDisabled(!structureVisible);
                auto& meshCommandService = mesh::application::GetMeshCommandService();

                for (const auto& entry : gridMeshes) {
                    bool visible = entry.visible;
                    std::string displayName = entry.name;
                    if (!displayName.empty()) {
                        displayName[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(displayName[0])));
                    }
                    std::string label = "Show " + displayName;
                    if (ImGui::Checkbox(label.c_str(), &visible)) {
                        if (visible) {
                            MeshDetail::Instance().SetUiVolumeMeshVisibility(true);
                            meshCommandService.ShowMesh(entry.id);
                        } else {
                            meshCommandService.HideMesh(entry.id);
                        }
                    }
                }

                ImGui::EndDisabled();
            }

            ImGui::TreePop();
            return;
        }

        if (ImGui::Checkbox("Show Isosurface", &m_showIsosurface)) {
            updateIsosurface();
        }

        if (m_showIsosurface && m_structureVisible) {
            ImGui::Indent();

            if (ImGui::Checkbox("Auto Value", &m_autoIsoValue)) {
                if (m_autoIsoValue) {
                    m_isoValueDirectInput = false;
                    m_isoValueDirectInputFocus = false;
                    calculateDefaultIsoValue();
                    updateIsosurface();
                }
            }

            ImGui::BeginDisabled(m_autoIsoValue);

            float absMax = std::max(std::abs(m_minValue), std::abs(m_maxValue));
            if (absMax > 0) {
                if (m_isoValueDirectInput) {
                    if (m_isoValueDirectInputFocus) {
                        ImGui::SetKeyboardFocusHere();
                        m_isoValueDirectInputFocus = false;
                    }

                    float isoInput = m_isoValue;
                    if (ImGui::InputFloat("Iso Value", &isoInput, 0.0f, 0.0f, "%.4e",
                                          ImGuiInputTextFlags_CharsScientific | ImGuiInputTextFlags_EnterReturnsTrue)) {
                        isoInput = std::clamp(isoInput, m_minValue, m_maxValue);
                        if (isoInput != m_isoValue) {
                            m_isoValue = isoInput;
                            updateIsosurface();
                        }
                        m_isoValueDirectInput = false;
                    } else if (ImGui::IsItemDeactivatedAfterEdit()) {
                        m_isoValueDirectInput = false;
                    }
                } else {
                    if (ImGui::SliderFloat("Iso Value", &m_isoValue, m_minValue, m_maxValue, "%.4e")) {
                        updateIsosurface();
                    }
                    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                        m_isoValueDirectInput = true;
                        m_isoValueDirectInputFocus = true;
                    }
                }

                const float valueSpan = (m_maxValue - m_minValue);
                if (valueSpan > 0.0f) {
                    float percentage = (m_isoValue - m_minValue) / valueSpan * 100.0f;
                    if (ImGui::SliderFloat("Level (%)", &percentage, 0.0f, 100.0f, "%.1f%%")) {
                        m_isoValue = m_minValue + valueSpan * percentage / 100.0f;
                        updateIsosurface();
                    }
                }
            }

            ImGui::EndDisabled();

            if (ImGui::ColorEdit4("Color##iso", m_isoColor,
                                  ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreview)) {
                storeIsoColorForLoadedGrid();
                updateIsosurface();
            }

            ImGui::Unindent();
        }

        ImGui::Spacing();

        if (ImGui::Checkbox("Show +/- Isosurfaces", &m_showMultipleIsosurfaces)) {
            rebuildMultipleIsosurfaces();
            if (m_showMultipleIsosurfaces) {
                SPDLOG_INFO("Multiple isosurfaces enabled");
            }
        }

        if (m_showMultipleIsosurfaces) {
            ImGui::Indent();

            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.3f, 0.1f, 0.1f, 1.0f));
            if (ImGui::SliderFloat("+ Value", &m_isoValuePositive, 0.0f, m_maxValue, "%.4e")) {
                rebuildMultipleIsosurfaces();
            }
            ImGui::PopStyleColor();

            if (ImGui::ColorEdit4("+ Color", m_isoColorPositive,
                                  ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreview)) {
                rebuildMultipleIsosurfaces();
            }

            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.1f, 0.3f, 1.0f));
            if (ImGui::SliderFloat("- Value", &m_isoValueNegative, m_minValue, 0.0f, "%.4e")) {
                rebuildMultipleIsosurfaces();
            }
            ImGui::PopStyleColor();

            if (ImGui::ColorEdit4("- Color", m_isoColorNegative,
                                  ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreview)) {
                rebuildMultipleIsosurfaces();
            }

            ImGui::Unindent();
        }

        ImGui::TreePop();
    }
}

void ChargeDensityUI::rebuildMultipleIsosurfaces() {
    if (!m_renderer) {
        return;
    }

    auto applyToRenderer = [&](infrastructure::ChargeDensityRenderer* renderer, bool gridVisible) {
        if (!renderer) {
            return;
        }
        renderer->clearIsosurfaces();
        if (!m_showMultipleIsosurfaces || !renderer->hasData() || !m_structureVisible || !gridVisible) {
            return;
        }

        if (m_isoValuePositive >= m_minValue && m_isoValuePositive <= m_maxValue) {
            renderer->addIsosurface(
                m_isoValuePositive,
                m_isoColorPositive[0], m_isoColorPositive[1],
                m_isoColorPositive[2], m_isoColorPositive[3]);
        }

        if (m_isoValueNegative >= m_minValue && m_isoValueNegative <= m_maxValue) {
            renderer->addIsosurface(
                m_isoValueNegative,
                m_isoColorNegative[0], m_isoColorNegative[1],
                m_isoColorNegative[2], m_isoColorNegative[3]);
        }
    };

    if (!m_loadedFromGrid || m_gridDataEntries.empty()) {
        applyToRenderer(m_renderer.get(), true);
        return;
    }

    const std::string activeKey = toLowerCopyForIsosurfacePanel(m_loadedFileName);
    applyToRenderer(m_renderer.get(), isSimpleGridVisibleByKey(activeKey));

    for (auto& [key, renderer] : m_auxRenderers) {
        applyToRenderer(renderer.get(), isSimpleGridVisibleByKey(key));
    }
}

}  // namespace ui
}  // namespace atoms

