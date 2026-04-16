#pragma once

namespace platform::persistence {

/**
 * @brief Browser-backed preference store for viewer settings.
 */
class ViewerPreferencesStore {
public:
    static void SaveBackgroundGradient(const char* key, const float* colorTop, const float* colorBottom);
    static void LoadBackgroundGradient(const char* key, float* colorTop, float* colorBottom);
};

} // namespace platform::persistence

