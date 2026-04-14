#pragma once

#include "../domain/mesh_repository.h"

// Standard library
#include <cstdint>
#include <functional>

class MeshDetail {
public:
    /**
     * @brief Compatibility accessor resolved via runtime-owned panel instance.
     * @note Phase 11 runtime shim. Remove this wrapper in Phase 12.
     */
    static MeshDetail& Instance();

    MeshDetail();
    ~MeshDetail();

    void Render(int32_t meshId, bool* openWindow = nullptr);
    void RenderVolumeControls(int32_t meshId, const char* tableId, bool showRenderMode = true);

    // Getters
    bool GetUiEdgeMeshVisibility() const { return m_UiEdgeMeshVisibility; }
    bool GetUiFaceMeshVisibility() const { return m_UiFaceMeshVisibility; }
    bool GetUiVolumeMeshVisibility() const { return m_UiVolumeMeshVisibility; }

    int GetUiVolumeRenderMode() const { return m_UiVolumeRenderMode; }
    int GetUiVolumeColorPreset() const { return m_UiVolumeColorPreset; }

    // Edge mesh setters
    void SetUiEdgeMeshColor(const double* color);
    void SetUiEdgeMeshOpacity(double opacity);
    void SetUiEdgeMeshVisibility(bool visibility) { m_UiEdgeMeshVisibility = visibility; }

    // Face mesh setters
    void SetUiFaceMeshColor(const double* color);
    void SetUiFaceMeshOpacity(double opacity);
    void SetUiFaceMeshEdgeColor(const double* color);
    void SetUiFaceMeshVisibility(bool visibility) { m_UiFaceMeshVisibility = visibility; }

    // Volume mesh setters
    void SetUiVolumeMeshColor(const double* color);
    void SetUiVolumeMeshOpacity(double opacity);
    void SetUiVolumeMeshEdgeColor(const double* color);
    void SetUiVolumeMeshVisibility(bool visibility) { m_UiVolumeMeshVisibility = visibility; }

    // Volume rendering setters
    void SetUiVolumeWindowLevel(double window, double level);
    void SetUiVolumeRenderOpacityMin(double opacity);
    void SetUiVolumeRenderOpacity(double opacity);
    void SetUiVolumeRenderMode(int mode) { m_UiVolumeRenderMode = mode; }
    void SetUiVolumeColorPreset(int preset) { m_UiVolumeColorPreset = preset; }
    void SetUiVolumeColorCurve(double midpoint, double sharpness);
    void SetUiVolumeSampleDistanceIndex(int index) { m_UiVolumeSampleDistanceIndex = index; }
    void SetUiVolumeQuality(int quality) { m_UiVolumeQuality = quality; }

    // Changed-flag setters
    void SetHasEdgeMeshColorChanged(bool hasChanged) { s_HasEdgeMeshColorChanged = hasChanged; }
    void SetHasEdgeMeshOpacityChanged(bool hasChanged) { s_HasEdgeMeshOpacityChanged = hasChanged; }

    void SetHasFaceMeshColorChanged(bool hasChanged) { s_HasFaceMeshColorChanged = hasChanged; }
    void SetHasFaceMeshOpacityChanged(bool hasChanged) { s_HasFaceMeshOpacityChanged = hasChanged; }
    void SetHasFaceMeshEdgeColorChanged(bool hasChanged) { s_HasFaceMeshEdgeColorChanged = hasChanged; }

    void SetHasVolumeMeshColorChanged(bool hasChanged) { s_HasVolumeMeshColorChanged = hasChanged; }
    void SetHasVolumeMeshOpacityChanged(bool hasChanged) { s_HasVolumeMeshOpacityChanged = hasChanged; }
    void SetHasVolumeMeshEdgeColorChanged(bool hasChanged) { s_HasVolumeMeshEdgeColorChanged = hasChanged; }

    void SetHasVolumeRenderOpacityMinChanged(bool hasChanged) { s_HasVolumeRenderOpacityMinChanged = hasChanged; }
    void SetHasVolumeRenderOpacityMaxChanged(bool hasChanged) { s_HasVolumeRenderOpacityMaxChanged = hasChanged; }
    void SetHasVolumeWindowLevelChanged(bool hasChanged) { s_HasVolumeWindowLevelChanged = hasChanged; }
    void SetHasVolumeColorCurveChanged(bool hasChanged) { s_HasVolumeColorCurveChanged = hasChanged; }
    void SetHasVolumeSampleDistanceChanged(bool hasChanged) { s_HasVolumeSampleDistanceChanged = hasChanged; }

private:
    void forEachTargetVolumeMesh(int32_t meshId, const std::function<void(Mesh*)>& func);
    void syncVolumeUiFromMesh(const Mesh& mesh);

    // Edge mesh UI state
    float m_UiEdgeMeshColor[3] { 0.0f, 0.0f, 0.0f };
    float m_UiEdgeMeshOpacity { 0.0f };
    bool m_UiEdgeMeshVisibility { false };

    // Face mesh UI state
    float m_UiFaceMeshColor[3] { 0.0f, 0.0f, 0.0f };
    float m_UiFaceMeshOpacity { 0.0f };
    float m_UiFaceMeshEdgeColor[3] { 0.0f, 0.0f, 0.0f };
    bool m_UiFaceMeshVisibility { false };

    // Volume mesh UI state
    float m_UiVolumeMeshColor[3] { 0.0f, 0.0f, 0.0f };
    float m_UiVolumeMeshOpacity { 0.0f };
    float m_UiVolumeMeshEdgeColor[3] { 0.0f, 0.0f, 0.0f };
    bool m_UiVolumeMeshVisibility { true };
    bool m_UiVolumeDirectApply { true };
    bool m_VolumeUseSharedSettings { true };
    int32_t m_LastVolumeControlMeshId { -1 };

    // Volume rendering UI state
    float m_UiVolumeWindow { 1.0f };
    float m_UiVolumeLevel { 0.5f };
    float m_UiVolumeRenderOpacityMin { 0.0f };
    float m_UiVolumeRenderOpacity { 0.0f };
    int m_UiVolumeRenderMode { 0 };
    int m_UiVolumeColorPreset { static_cast<int>(Mesh::VolumeColorPreset::Rainbow) };
    float m_UiVolumeColorMidpoint { 0.5f };
    float m_UiVolumeColorSharpness { 0.0f };
    int m_UiVolumeSampleDistanceIndex { 2 };
    int m_UiVolumeQuality { 2 };

    // Static changed flags
    static bool s_HasEdgeMeshColorChanged;
    static bool s_HasEdgeMeshOpacityChanged;

    static bool s_HasFaceMeshColorChanged;
    static bool s_HasFaceMeshOpacityChanged;
    static bool s_HasFaceMeshEdgeColorChanged;

    static bool s_HasVolumeMeshColorChanged;
    static bool s_HasVolumeMeshOpacityChanged;
    static bool s_HasVolumeMeshEdgeColorChanged;

    static bool s_HasVolumeRenderOpacityMinChanged;
    static bool s_HasVolumeRenderOpacityMaxChanged;
    static bool s_HasVolumeWindowLevelChanged;
    static bool s_HasVolumeColorCurveChanged;
    static bool s_HasVolumeSampleDistanceChanged;

    // Basic table rows
    void renderTableRow(const char* item, int32_t value);
    void renderTableRow(const char* item, const char* value = nullptr);
    void renderTableRow(const char* item, double value);

    // Edge mesh table rows
    void renderTableRowEdgeMeshColor(int32_t meshId, const double* color);
    void renderTableRowEdgeMeshOpacity(int32_t meshId, double opacity);
    void renderTableRowEdgeMeshVisibility(int32_t meshId, bool visibility);

    // Face mesh table rows
    void renderTableRowFaceMeshColor(int32_t meshId, const double* color);
    void renderTableRowFaceMeshOpacity(int32_t meshId, double opacity);
    void renderTableRowFaceMeshEdgeColor(int32_t meshId, const double* color);
    void renderTableRowFaceMeshVisibility(int32_t meshId, bool visibility);

    // Volume mesh table rows
    void renderTableRowVolumeMeshColor(int32_t meshId, const double* color);
    void renderTableRowVolumeMeshOpacity(int32_t meshId, double opacity);
    void renderTableRowVolumeMeshEdgeColor(int32_t meshId, const double* color);
    void renderTableRowVolumeMeshVisibility(int32_t meshId, bool visibility, bool linkToTree);
    void renderTableRowVolumeDirectApply();

    // Volume rendering rows
    void renderTableRowVolumeRenderMode(int32_t meshId, bool volumeAvailable);
    void renderTableRowVolumeWindowLevel(int32_t meshId);
    void renderTableRowVolumeRenderOpacityMin(int32_t meshId, double opacity);
    void renderTableRowVolumeRenderOpacity(int32_t meshId, double opacity);
    void renderTableRowVolumeColorPreset(int32_t meshId, bool volumeAvailable);
    void renderTableRowVolumeColorCurve(int32_t meshId);
    void renderTableRowVolumeSampleDistance(int32_t meshId);
    void renderTableRowVolumeQuality(int32_t meshId);

    void syncSharedVolumeUiFromSettings(const mesh::domain::MeshRepository::VolumeDisplaySettings& settings);
};


