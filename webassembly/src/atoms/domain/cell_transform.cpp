#include "cell_transform.h"

#include "../../config/log_config.h"

#include <cmath>

namespace atoms::domain {

void calculateInverseMatrix(const float matrix[3][3], float inverse[3][3]) {
    const float det =
        matrix[0][0] * (matrix[1][1] * matrix[2][2] - matrix[1][2] * matrix[2][1]) -
        matrix[0][1] * (matrix[1][0] * matrix[2][2] - matrix[1][2] * matrix[2][0]) +
        matrix[0][2] * (matrix[1][0] * matrix[2][1] - matrix[1][1] * matrix[2][0]);

    if (std::abs(det) < 1e-6f) {
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                inverse[i][j] = (i == j) ? 1.0f : 0.0f;
            }
        }
        SPDLOG_WARN("Matrix is singular or near-singular. Inverse calculation failed.");
        return;
    }

    const float invDet = 1.0f / det;

    inverse[0][0] = invDet * (matrix[1][1] * matrix[2][2] - matrix[1][2] * matrix[2][1]);
    inverse[0][1] = invDet * (matrix[0][2] * matrix[2][1] - matrix[0][1] * matrix[2][2]);
    inverse[0][2] = invDet * (matrix[0][1] * matrix[1][2] - matrix[0][2] * matrix[1][1]);

    inverse[1][0] = invDet * (matrix[1][2] * matrix[2][0] - matrix[1][0] * matrix[2][2]);
    inverse[1][1] = invDet * (matrix[0][0] * matrix[2][2] - matrix[0][2] * matrix[2][0]);
    inverse[1][2] = invDet * (matrix[0][2] * matrix[1][0] - matrix[0][0] * matrix[1][2]);

    inverse[2][0] = invDet * (matrix[1][0] * matrix[2][1] - matrix[1][1] * matrix[2][0]);
    inverse[2][1] = invDet * (matrix[0][1] * matrix[2][0] - matrix[0][0] * matrix[2][1]);
    inverse[2][2] = invDet * (matrix[0][0] * matrix[1][1] - matrix[0][1] * matrix[1][0]);
}

void cartesianToFractional(
    const float cartesian[3],
    float fractional[3],
    const float invmatrix[3][3]) {
    for (int i = 0; i < 3; ++i) {
        fractional[i] = 0.0f;
        for (int j = 0; j < 3; ++j) {
            fractional[i] += cartesian[j] * invmatrix[j][i];
        }
    }
}

void fractionalToCartesian(
    const float fractional[3],
    float cartesian[3],
    const float matrix[3][3]) {
    for (int i = 0; i < 3; ++i) {
        cartesian[i] = 0.0f;
        for (int j = 0; j < 3; ++j) {
            cartesian[i] += fractional[j] * matrix[j][i];
        }
    }
}

} // namespace atoms::domain
