/**
 * @file structure_service_types.h
 * @brief Shared DTO types for structure application services.
 */
#pragma once

#include <cstdint>
#include <string>

namespace structure {
namespace application {

/**
 * @brief Read model entry for structure list UI.
 */
struct StructureEntryView {
    int32_t id = -1;
    std::string name;
    bool visible = true;
};

} // namespace application
} // namespace structure

