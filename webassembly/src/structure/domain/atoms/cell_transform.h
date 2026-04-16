#pragma once

namespace atoms::domain {

void calculateInverseMatrix(const float matrix[3][3], float inverse[3][3]);
void cartesianToFractional(
    const float cartesian[3],
    float fractional[3],
    const float invmatrix[3][3]);
void fractionalToCartesian(
    const float fractional[3],
    float cartesian[3],
    const float matrix[3][3]);

} // namespace atoms::domain
