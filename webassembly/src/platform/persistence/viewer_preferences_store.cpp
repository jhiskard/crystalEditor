#include "viewer_preferences_store.h"

#include <emscripten/emscripten.h>

namespace platform::persistence {

void ViewerPreferencesStore::SaveBackgroundGradient(
    const char* key,
    const float* colorTop,
    const float* colorBottom) {
    if (key == nullptr || colorTop == nullptr || colorBottom == nullptr) {
        return;
    }

    EM_ASM({
        const bgColorKey = UTF8ToString($0);
        const color = new Array($1, $2, $3, $4, $5, $6);
        localStorage.setItem(bgColorKey, JSON.stringify(color));
    }, key, colorTop[0], colorTop[1], colorTop[2], colorBottom[0], colorBottom[1], colorBottom[2]);
}

void ViewerPreferencesStore::LoadBackgroundGradient(
    const char* key,
    float* colorTop,
    float* colorBottom) {
    if (key == nullptr || colorTop == nullptr || colorBottom == nullptr) {
        return;
    }

    EM_ASM({
        const bgColorKey = UTF8ToString($0);
        const color = JSON.parse(localStorage.getItem(bgColorKey));
        if (color && color.length === 6) {
            setValue($1, color[0], 'float');
            setValue($2, color[1], 'float');
            setValue($3, color[2], 'float');
            setValue($4, color[3], 'float');
            setValue($5, color[4], 'float');
            setValue($6, color[5], 'float');
        }
    }, key, &colorTop[0], &colorTop[1], &colorTop[2], &colorBottom[0], &colorBottom[1], &colorBottom[2]);
}

} // namespace platform::persistence

