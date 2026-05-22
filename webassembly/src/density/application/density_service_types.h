/**
 * @file density_service_types.h
 * @brief Application-layer DTOs for charge-density UI state.
 */
#pragma once

#include <string>

namespace density {
namespace application {

struct SimpleGridEntry {
    std::string name;
    bool visible = false;
};

struct SliceGridEntry {
    std::string name;
    bool visible = false;
};

} // namespace application
} // namespace density
