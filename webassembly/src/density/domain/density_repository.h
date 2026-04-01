/**
 * @file density_repository.h
 * @brief Density domain repository placeholder for Phase 8 extraction.
 */
#pragma once

#include <cstdint>

namespace density {
namespace domain {

/**
 * @brief Lightweight density context snapshot.
 */
struct DensityContext {
    int32_t structureId = -1;
    bool hasDensity = false;
    bool visible = false;
};

/**
 * @brief Read-only repository interface for density context queries.
 */
class DensityRepository {
public:
    virtual ~DensityRepository() = default;
    virtual DensityContext GetContext() const = 0;
};

} // namespace domain
} // namespace density

