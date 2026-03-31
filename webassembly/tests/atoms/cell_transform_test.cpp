#include "../test_harness.h"
#include "../../src/atoms/domain/cell_transform.h"

namespace {

void assertMatrixEntry(const float matrix[3][3], int row, int col, double expected) {
    ASSERT_NEAR(matrix[row][col], expected, 1e-6);
}

} // namespace

TEST_CASE(cell_transform_calculates_inverse_for_diagonal_matrix) {
    const float matrix[3][3] = {
        { 2.0f, 0.0f, 0.0f },
        { 0.0f, 4.0f, 0.0f },
        { 0.0f, 0.0f, 5.0f },
    };
    float inverse[3][3] = {};

    atoms::domain::calculateInverseMatrix(matrix, inverse);

    assertMatrixEntry(inverse, 0, 0, 0.5);
    assertMatrixEntry(inverse, 1, 1, 0.25);
    assertMatrixEntry(inverse, 2, 2, 0.2);
}

TEST_CASE(cell_transform_round_trips_cartesian_fractional_coordinates) {
    const float matrix[3][3] = {
        { 3.0f, 0.0f, 0.0f },
        { 1.0f, 4.0f, 0.0f },
        { 0.5f, 1.5f, 5.0f },
    };
    float inverse[3][3] = {};
    const float cartesian[3] = { 1.5f, 2.25f, 3.75f };
    float fractional[3] = {};
    float restored[3] = {};

    atoms::domain::calculateInverseMatrix(matrix, inverse);
    atoms::domain::cartesianToFractional(cartesian, fractional, inverse);
    atoms::domain::fractionalToCartesian(fractional, restored, matrix);

    ASSERT_NEAR(restored[0], cartesian[0], 1e-5);
    ASSERT_NEAR(restored[1], cartesian[1], 1e-5);
    ASSERT_NEAR(restored[2], cartesian[2], 1e-5);
}

TEST_CASE(cell_transform_falls_back_to_identity_for_singular_matrix) {
    const float matrix[3][3] = {
        { 1.0f, 2.0f, 3.0f },
        { 2.0f, 4.0f, 6.0f },
        { 0.0f, 0.0f, 0.0f },
    };
    float inverse[3][3] = {};

    atoms::domain::calculateInverseMatrix(matrix, inverse);

    assertMatrixEntry(inverse, 0, 0, 1.0);
    assertMatrixEntry(inverse, 0, 1, 0.0);
    assertMatrixEntry(inverse, 1, 1, 1.0);
    assertMatrixEntry(inverse, 2, 2, 1.0);
}
