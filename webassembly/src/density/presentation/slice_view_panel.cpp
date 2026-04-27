#include "../../density/presentation/charge_density_ui.h"

#include "../../workspace/runtime/legacy_atoms_runtime.h"
#include "../../density/infrastructure/charge_density_renderer.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#define GLFW_INCLUDE_ES3
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <algorithm>
#include <cmath>
#include <unordered_set>

namespace atoms {
namespace ui {

#ifdef __EMSCRIPTEN__
EM_JS(void, download_slice_png, (const uint8_t* data, int width, int height, const char* filename, int nameLength), {
    const heap = (typeof HEAPU8 !== 'undefined') ? HEAPU8 :
                 ((typeof Module !== 'undefined' && Module.HEAPU8) ? Module.HEAPU8 :
                 ((typeof VtkModule !== 'undefined' && VtkModule.HEAPU8) ? VtkModule.HEAPU8 : null));
    if (!heap) {
        console.error("HEAPU8 is not available for PNG download.");
        return;
    }
    const w = Number(width);
    const h = Number(height);
    if (!Number.isFinite(w) || !Number.isFinite(h) || w <= 0 || h <= 0) {
        console.error("Invalid slice size for PNG download.");
        return;
    }
    const start = (typeof data === 'bigint') ? Number(data) : data;
    const size = w * h * 4;
    if (!start || !Number.isFinite(start) || !Number.isFinite(size) || size <= 0) {
        console.error("Invalid slice data for PNG download.");
        return;
    }
    const src = heap.subarray(start, start + size);
    const pixels = new Uint8ClampedArray(src);
    const canvas = document.createElement('canvas');
    canvas.width = w;
    canvas.height = h;
    const ctx = canvas.getContext('2d');
    if (!ctx) {
        console.error("Canvas 2D context is unavailable.");
        return;
    }
    const imageData = ctx.createImageData(w, h);
    imageData.data.set(pixels);
    ctx.putImageData(imageData, 0, 0);
    const name = (() => {
        if (!filename) {
            return 'slice.png';
        }
        const ptr = (typeof filename === 'bigint') ? Number(filename) : filename;
        if (!ptr || !Number.isFinite(ptr)) {
            return 'slice.png';
        }
        const len = Number(nameLength);
        if (!Number.isFinite(len) || len <= 0) {
            return 'slice.png';
        }
        const end = Math.min(ptr + len, heap.length);
        if (end <= ptr) {
            return 'slice.png';
        }
        try {
            return new TextDecoder('utf-8').decode(heap.subarray(ptr, end)) || 'slice.png';
        } catch (e) {
            return 'slice.png';
        }
    })();
    canvas.toBlob((blob) => {
        const link = document.createElement('a');
        link.href = URL.createObjectURL(blob);
        link.download = name || 'slice.png';
        document.body.appendChild(link);
        link.click();
        document.body.removeChild(link);
        setTimeout(() => URL.revokeObjectURL(link.href), 1000);
    }, 'image/png');
});
#endif

void ChargeDensityUI::renderSliceControls() {
    bool previewDirty = false;
    const std::string activeSliceKey = getActiveSliceSettingsKey();

    if (ImGui::Checkbox("Show Slice", &m_showSlice)) {
        updateSlice();
        previewDirty = true;
    }
    
    ImGui::BeginDisabled(!m_showSlice);
    ImGui::Indent();
    
    const int millerPlaneValue =
        static_cast<int>(infrastructure::ChargeDensityRenderer::SlicePlane::Miller);

    // Plane preset + Miller mode
    const char* planeNames[] = {
        "XY (Z=const)",
        "XZ (Y=const)",
        "YZ (X=const)",
        "Define plane by Miller indices"
    };
    if (ImGui::Combo("Plane", &m_slicePlane, planeNames, 4)) {
        if (m_renderer) {
            m_renderer->setSlicePlane(
                static_cast<infrastructure::ChargeDensityRenderer::SlicePlane>(m_slicePlane)
            );
            m_renderer->setSliceMillerIndices(m_sliceMillerH, m_sliceMillerK, m_sliceMillerL);
        }
        previewDirty = true;
    }

    const bool isMillerMode = (m_slicePlane == millerPlaneValue);
    if (isMillerMode) {
        bool millerChanged = false;
        millerChanged |= ImGui::SliderInt("h", &m_sliceMillerH, 0, 6);
        millerChanged |= ImGui::SliderInt("k", &m_sliceMillerK, 0, 6);
        millerChanged |= ImGui::SliderInt("l", &m_sliceMillerL, 0, 6);
        if (millerChanged) {
            if (m_renderer) {
                m_renderer->setSliceMillerIndices(m_sliceMillerH, m_sliceMillerK, m_sliceMillerL);
            }
            previewDirty = true;
        }
        if (m_sliceMillerH == 0 && m_sliceMillerK == 0 && m_sliceMillerL == 0) {
            ImGui::TextColored(ImVec4(1.0f, 0.55f, 0.35f, 1.0f),
                               "h, k, l cannot all be 0.");
        }
    }
    
    // Slice position
    const char* axisLabels[] = { "Z Position", "Y Position", "X Position" };
    const char* positionLabel = isMillerMode ? "Position" : axisLabels[m_slicePlane];
    if (ImGui::SliderFloat(positionLabel, &m_slicePosition, 0.0f, 1.0f, "%.3f")) {
        if (m_renderer) {
            m_renderer->setSlicePosition(m_slicePosition);
        }
        previewDirty = true;
    }
    
    if (isMillerMode) {
        const int maxDim = std::max({m_gridShape[0], m_gridShape[1], m_gridShape[2]});
        const int maxIndex = std::max(0, maxDim - 1);
        int gridIndex = static_cast<int>(std::round(m_slicePosition * static_cast<float>(maxIndex)));
        gridIndex = std::clamp(gridIndex, 0, maxIndex);
        if (ImGui::SliderInt("Grid Index", &gridIndex, 0, maxIndex)) {
            if (maxIndex > 0) {
                m_slicePosition = static_cast<float>(gridIndex) / static_cast<float>(maxIndex);
            } else {
                m_slicePosition = 0.0f;
            }
            if (m_renderer) {
                m_renderer->setSlicePosition(m_slicePosition);
            }
            previewDirty = true;
        }
    } else {
        int gridIndex = 0;
        switch (m_slicePlane) {
            case 0: gridIndex = static_cast<int>(m_slicePosition * m_gridShape[2]); break;
            case 1: gridIndex = static_cast<int>(m_slicePosition * m_gridShape[1]); break;
            case 2: gridIndex = static_cast<int>(m_slicePosition * m_gridShape[0]); break;
            default: break;
        }
        const int maxIndex = (m_slicePlane == 0) ? std::max(0, m_gridShape[2] - 1)
                           : (m_slicePlane == 1) ? std::max(0, m_gridShape[1] - 1)
                                                 : std::max(0, m_gridShape[0] - 1);
        if (ImGui::SliderInt("Grid Index", &gridIndex, 0, maxIndex)) {
            m_slicePosition = static_cast<float>(gridIndex) / static_cast<float>(maxIndex + 1);
            if (m_renderer) {
                m_renderer->setSlicePosition(m_slicePosition);
            }
            previewDirty = true;
        }
    }
    
    ImGui::Spacing();
    
    // ???????????
    if (ImGui::Checkbox("Auto Range", &m_autoRange)) {
        if (m_autoRange) {
            m_displayMin = m_minValue;
            m_displayMax = m_maxValue;
        }
        storeSliceDisplaySettingsForLoadedGrid();
        if (m_renderer) {
            syncSliceSettingsForRenderer(m_renderer.get(), activeSliceKey);
        }
        previewDirty = true;
    }
    
    ImGui::BeginDisabled(m_autoRange);
    
    const float rangeLower = std::min(m_minValue, m_maxValue);
    const float rangeUpper = std::max(m_minValue, m_maxValue);
    const ImGuiSliderFlags rangeSliderFlags = ImGuiSliderFlags_AlwaysClamp;
    bool applyRangeNow = false;

    if (ImGui::SliderFloat("Display Min", &m_displayMin,
                           rangeLower, m_displayMax, "%.3f", rangeSliderFlags)) {
        const bool isTextEditing = ImGui::IsItemActive() && !ImGui::IsMouseDown(ImGuiMouseButton_Left);
        if (!isTextEditing) {
            applyRangeNow = true;
        }
    }
    const bool minEditFinished = ImGui::IsItemDeactivatedAfterEdit();

    if (ImGui::SliderFloat("Display Max", &m_displayMax,
                           m_displayMin, rangeUpper, "%.3f", rangeSliderFlags)) {
        const bool isTextEditing = ImGui::IsItemActive() && !ImGui::IsMouseDown(ImGuiMouseButton_Left);
        if (!isTextEditing) {
            applyRangeNow = true;
        }
    }
    const bool maxEditFinished = ImGui::IsItemDeactivatedAfterEdit();

    if (minEditFinished || maxEditFinished) {
        applyRangeNow = true;
    }

    if (applyRangeNow && m_renderer) {
        if (m_displayMin > m_displayMax) {
            std::swap(m_displayMin, m_displayMax);
        }
        storeSliceDisplaySettingsForLoadedGrid();
        syncSliceSettingsForRenderer(m_renderer.get(), activeSliceKey);
        previewDirty = true;
    }
    
    ImGui::EndDisabled();
    
    ImGui::Unindent();
    ImGui::EndDisabled();

    if (previewDirty) {
        markSlicePreviewCachesDirty();
    }

    if (m_showSlice && m_slicePreviewDirty) {
        updateSlicePreview();
    }
}

void ChargeDensityUI::renderSlicePreview() {
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Slice Preview");
    const auto targets = collectSliceRenderTargets();
    const bool hasSliceTargets = !targets.empty();
    const float buttonWidth = ImGui::CalcTextSize("PNG").x + ImGui::GetStyle().FramePadding.x * 2.0f;
    float rightX = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - buttonWidth;
    ImGui::SameLine();
    ImGui::SetCursorPosX(rightX);
    ImGui::BeginDisabled(!m_showSlice || !hasSliceTargets);
    if (ImGui::SmallButton("PNG")) {
#ifdef __EMSCRIPTEN__
        infrastructure::ChargeDensityRenderer* pngRenderer = nullptr;
        std::string pngKey;
        if (!m_slicePopupKey.empty()) {
            auto popupTargetIt = std::find_if(targets.begin(), targets.end(),
                [&](const SliceRenderTarget& target) {
                    return target.key == m_slicePopupKey;
                });
            if (popupTargetIt != targets.end()) {
                pngRenderer = popupTargetIt->renderer;
                pngKey = popupTargetIt->key;
            }
        }
        if (!pngRenderer && !targets.empty()) {
            if (m_loadedFromGrid) {
                auto visibleIt = std::find_if(targets.begin(), targets.end(),
                    [&](const SliceRenderTarget& target) {
                        return isSliceGridVisibleByKey(target.key);
                    });
                if (visibleIt != targets.end()) {
                    pngRenderer = visibleIt->renderer;
                    pngKey = visibleIt->key;
                }
            }
        }
        if (!pngRenderer && !targets.empty()) {
            pngRenderer = targets.front().renderer;
            pngKey = targets.front().key;
        }
        if (pngRenderer) {
            syncSliceSettingsForRenderer(pngRenderer, pngKey);
        }
        std::string baseName;
        if (m_atomsTemplate) {
            const std::string& loadedName = m_atomsTemplate->GetLoadedFileName();
            if (!loadedName.empty()) {
                baseName = loadedName;
            }
        }
        if (baseName.empty()) {
            baseName = m_loadedFileName;
        }
        if (baseName.empty() && m_atomsTemplate) {
            int32_t structureId = m_atomsTemplate->GetCurrentStructureId();
            for (const auto& entry : m_atomsTemplate->GetStructures()) {
                if (entry.id == structureId && !entry.name.empty()) {
                    baseName = entry.name;
                    break;
                }
            }
        }
        if (baseName.empty()) {
            baseName = "slice";
        }
        m_sliceDownloadName = baseName + ".slice.png";
        bool hasDownloadImage = false;
        if (pngRenderer) {
                hasDownloadImage = pngRenderer->captureRenderedSliceImage(
                    m_sliceDownloadPixels,
                    m_sliceDownloadWidth,
                    m_sliceDownloadHeight);
            if (!hasDownloadImage) {
                hasDownloadImage = buildSliceImageForRenderer(
                    pngRenderer,
                    pngKey,
                    2000,
                    2000,
                    4096,
                    m_sliceDownloadPixels,
                    m_sliceDownloadWidth,
                    m_sliceDownloadHeight);
            }
        }
        if (hasDownloadImage) {
            download_slice_png(m_sliceDownloadPixels.data(),
                               m_sliceDownloadWidth,
                               m_sliceDownloadHeight,
                               m_sliceDownloadName.c_str(),
                               static_cast<int>(m_sliceDownloadName.size()));
        }
#endif
    }
    ImGui::EndDisabled();

    if (!m_showSlice) {
        ImGui::TextDisabled("Slice is hidden");
        return;
    }

    if (!hasSliceTargets) {
        ImGui::TextDisabled("Preview not available");
        return;
    }

    if (m_slicePreviewDirty) {
        updateSlicePreview();
    }

    const ImVec2 avail = ImGui::GetContentRegionAvail();
    const float targetWidth = avail.x > 0.0f ? avail.x : 1.0f;

    for (const auto& target : targets) {
        auto it = m_slicePreviewCaches.find(target.key);
        if (it == m_slicePreviewCaches.end()) {
            continue;
        }
        SlicePreviewCache& cache = it->second;
        if (cache.dirty) {
            updateSlicePreviewForTarget(target);
        }
        if (!cache.ready || cache.texture == 0 || cache.width <= 0 || cache.height <= 0) {
            continue;
        }

        float aspect = static_cast<float>(cache.width) / static_cast<float>(cache.height);
        float width = targetWidth;
        float height = width / aspect;
        const float maxHeight = 260.0f;
        if (height > maxHeight) {
            height = maxHeight;
            width = height * aspect;
        }

        ImGui::Image((ImTextureID)(uintptr_t)cache.texture,
                     ImVec2(width, height),
                     cache.flipVerticallyWhenRendering ? ImVec2(0.0f, 1.0f) : ImVec2(0.0f, 0.0f),
                     cache.flipVerticallyWhenRendering ? ImVec2(1.0f, 0.0f) : ImVec2(1.0f, 1.0f));
        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            m_slicePopupKey = target.key;
            ImGui::OpenPopup(("Slice Original##" + target.key).c_str());
        }
        ImGui::TextDisabled("%s", target.label.c_str());

        const std::string popupId = "Slice Original##" + target.key;
        const ImVec2 displaySize = ImGui::GetIO().DisplaySize;
        const float popupMargin = 20.0f;
        const float availableWidth = std::max(0.0f, displaySize.x - popupMargin * 2.0f);
        const float popupWidth = std::max(320.0f, availableWidth * 0.8f);
        const float popupHeight = std::max(240.0f, displaySize.y - popupMargin * 2.0f);
        const float popupX = std::max(popupMargin, (displaySize.x - popupWidth) * 0.5f);
        ImGui::SetNextWindowPos(ImVec2(popupX, popupMargin), ImGuiCond_Appearing);
        ImGui::SetNextWindowSize(ImVec2(popupWidth, popupHeight), ImGuiCond_Appearing);
        if (ImGui::BeginPopupModal(popupId.c_str(), nullptr)) {
            ImGui::Text("%s", target.label.c_str());
            ImGui::Separator();

            const float buttonHeight = ImGui::GetFrameHeightWithSpacing();
            ImVec2 imageArea = ImGui::GetContentRegionAvail();
            imageArea.y = std::max(80.0f, imageArea.y - buttonHeight);
            const std::string childId = "##SliceOriginalFit" + target.key;
            if (ImGui::BeginChild(childId.c_str(), imageArea, true,
                                  ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
                const ImVec2 avail = ImGui::GetContentRegionAvail();
                const ImVec2 framebufferScale = ImGui::GetIO().DisplayFramebufferScale;
                const float dprX = std::max(1.0f, framebufferScale.x);
                const float dprY = std::max(1.0f, framebufferScale.y);
                const int requestedWidth = std::max(1, static_cast<int>(std::round(avail.x * dprX)));
                const int requestedHeight = std::max(1, static_cast<int>(std::round(avail.y * dprY)));

                SlicePreviewCache& popupCache = m_slicePopupCaches[target.key];
                const bool popupSizeChanged = popupCache.requestedWidth != requestedWidth ||
                                              popupCache.requestedHeight != requestedHeight;
                if (popupSizeChanged || popupCache.dirty || !popupCache.ready || popupCache.texture == 0) {
                    popupCache.ready = false;
                    popupCache.dirty = false;
                    popupCache.requestedWidth = requestedWidth;
                    popupCache.requestedHeight = requestedHeight;
                    if (target.renderer) {
                        syncSliceSettingsForRenderer(target.renderer, target.key);
                        bool captured = target.renderer->captureRenderedSliceImage(
                            popupCache.pixels,
                            popupCache.width,
                            popupCache.height);
                        popupCache.flipVerticallyWhenRendering = !captured;
                        if (!captured) {
                            captured = buildSliceImageForRenderer(target.renderer,
                                                                  target.key,
                                                                  requestedWidth,
                                                                  requestedHeight,
                                                                  4096,
                                                                  popupCache.pixels,
                                                                  popupCache.width,
                                                                  popupCache.height);
                            popupCache.flipVerticallyWhenRendering = true;
                        }
                        if (captured) {
                            uploadSlicePreviewTexture(popupCache);
                            popupCache.ready = (popupCache.texture != 0);
                        }
                    }
                }

                const SlicePreviewCache* displayCache = &popupCache;
                if (!popupCache.ready || popupCache.texture == 0 ||
                    popupCache.width <= 0 || popupCache.height <= 0) {
                    displayCache = &cache;
                }

                const float srcW = std::max(1.0f, static_cast<float>(displayCache->width));
                const float srcH = std::max(1.0f, static_cast<float>(displayCache->height));
                const float scaleX = avail.x / srcW;
                const float scaleY = avail.y / srcH;
                const float scale = std::max(0.01f, std::min(scaleX, scaleY));
                const float drawW = srcW * scale;
                const float drawH = srcH * scale;

                const float offsetX = std::max(0.0f, (avail.x - drawW) * 0.5f);
                const float offsetY = std::max(0.0f, (avail.y - drawH) * 0.5f);
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offsetY);

                ImGui::Image((ImTextureID)(uintptr_t)displayCache->texture,
                             ImVec2(drawW, drawH),
                             displayCache->flipVerticallyWhenRendering ? ImVec2(0.0f, 1.0f) : ImVec2(0.0f, 0.0f),
                             displayCache->flipVerticallyWhenRendering ? ImVec2(1.0f, 0.0f) : ImVec2(1.0f, 1.0f));
            }
            ImGui::EndChild();
            if (ImGui::Button("Close")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::Spacing();
    }
}

void ChargeDensityUI::updateSlicePreviewForTarget(const SliceRenderTarget& target) {
    if (!target.renderer) {
        return;
    }

    syncSliceSettingsForRenderer(target.renderer, target.key);
    SlicePreviewCache& cache = m_slicePreviewCaches[target.key];
    cache.dirty = false;
    cache.ready = false;
    cache.requestedWidth = 0;
    cache.requestedHeight = 0;

    if (!buildSliceImageForRenderer(target.renderer,
                                    target.key,
                                    0,
                                    0,
                                    1024,
                                    cache.pixels,
                                    cache.width,
                                    cache.height)) {
        return;
    }

    cache.flipVerticallyWhenRendering = true;
    uploadSlicePreviewTexture(cache);
    cache.ready = (cache.texture != 0);
}

void ChargeDensityUI::updateSlicePreview() {
    if (!m_showSlice) {
        m_slicePreviewDirty = false;
        return;
    }

    const auto targets = collectSliceRenderTargets();
    std::unordered_set<std::string> activeKeys;
    activeKeys.reserve(targets.size());
    for (const auto& target : targets) {
        activeKeys.insert(target.key);
    }

    for (auto it = m_slicePreviewCaches.begin(); it != m_slicePreviewCaches.end();) {
        if (activeKeys.find(it->first) == activeKeys.end()) {
            if (it->second.texture != 0) {
                glDeleteTextures(1, &it->second.texture);
            }
            it = m_slicePreviewCaches.erase(it);
        } else {
            ++it;
        }
    }
    for (auto it = m_slicePopupCaches.begin(); it != m_slicePopupCaches.end();) {
        if (activeKeys.find(it->first) == activeKeys.end()) {
            if (it->second.texture != 0) {
                glDeleteTextures(1, &it->second.texture);
            }
            it = m_slicePopupCaches.erase(it);
        } else {
            ++it;
        }
    }

    for (const auto& target : targets) {
        SlicePreviewCache& cache = m_slicePreviewCaches[target.key];
        if (m_slicePreviewDirty) {
            cache.dirty = true;
        }
        if (cache.dirty || !cache.ready || cache.texture == 0) {
            updateSlicePreviewForTarget(target);
        }
    }

    m_slicePreviewDirty = false;
}

}  // namespace ui
}  // namespace atoms






