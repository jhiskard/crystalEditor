/**
 * @file structure_tree_view_model.h
 * @brief Structure section presentation model primitives.
 */
#pragma once

#include <cstdint>
#include <string>

namespace structure {
namespace presentation {

/**
 * @brief Flat row model for structure tree presentation.
 */
struct StructureTreeRow {
    int32_t structureId = -1;
    std::string displayName;
    bool visible = true;
};

} // namespace presentation
} // namespace structure

