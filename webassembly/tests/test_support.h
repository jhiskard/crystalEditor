#pragma once

#include "test_config.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace wb::tests {

inline std::filesystem::path fixturePath(const std::string& relativePath) {
    return std::filesystem::path(WB_TEST_FIXTURE_DIR) / relativePath;
}

inline std::string readFixtureText(const std::string& relativePath) {
    const auto path = fixturePath(relativePath);
    std::ifstream stream(path, std::ios::binary);
    if (!stream.is_open()) {
        throw std::runtime_error("Failed to open fixture: " + path.string());
    }

    std::ostringstream buffer;
    buffer << stream.rdbuf();
    return buffer.str();
}

} // namespace wb::tests
