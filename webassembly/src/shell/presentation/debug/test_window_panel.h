#pragma once

// Standard library
#include <vector>

/**
 * @brief Debug test panel.
 */
class TestWindow {
public:
    /**
     * @brief Compatibility accessor resolved via runtime-owned panel instance.
     * @note Phase 11 runtime shim. Remove this wrapper in Phase 12.
     */
    static TestWindow& Instance();

    TestWindow();
    ~TestWindow();

    void Render(bool* openWindow = nullptr);

private:
    std::vector<void*> m_TestPtrs;
};
