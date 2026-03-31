#include "../test_harness.h"
#include "../../src/atoms/domain/measurement_geometry.h"

#include <array>

TEST_CASE(measurement_geometry_calculates_distance) {
    const float lhs[3] = { 0.0f, 0.0f, 0.0f };
    const float rhs[3] = { 1.0f, 2.0f, 2.0f };

    const double distance = atoms::domain::calcDistance(lhs, rhs);

    ASSERT_NEAR(distance, 3.0, 1e-6);
}

TEST_CASE(measurement_geometry_builds_angle_geometry) {
    const float point1[3] = { 1.0f, 0.0f, 0.0f };
    const float point2[3] = { 0.0f, 0.0f, 0.0f };
    const float point3[3] = { 0.0f, 1.0f, 0.0f };
    double angleDeg = 0.0;
    std::array<double, 3> arcStart {};
    std::array<double, 3> arcEnd {};
    std::array<double, 3> arcCenter {};
    std::array<double, 3> textPos {};

    const bool success = atoms::domain::computeAngleMeasurementGeometry(
        point1, point2, point3, 1.0, angleDeg, arcStart, arcEnd, arcCenter, textPos);

    ASSERT_TRUE(success);
    ASSERT_NEAR(angleDeg, 90.0, 1e-6);
    ASSERT_NEAR(arcCenter[0], 0.0, 1e-6);
    ASSERT_NEAR(arcCenter[1], 0.0, 1e-6);
    ASSERT_TRUE(textPos[0] > 0.0);
    ASSERT_TRUE(textPos[1] > 0.0);
}

TEST_CASE(measurement_geometry_rejects_degenerate_angle) {
    const float point1[3] = { 0.0f, 0.0f, 0.0f };
    const float point2[3] = { 0.0f, 0.0f, 0.0f };
    const float point3[3] = { 0.0f, 1.0f, 0.0f };
    double angleDeg = 0.0;
    std::array<double, 3> arcStart {};
    std::array<double, 3> arcEnd {};
    std::array<double, 3> arcCenter {};
    std::array<double, 3> textPos {};

    const bool success = atoms::domain::computeAngleMeasurementGeometry(
        point1, point2, point3, 1.0, angleDeg, arcStart, arcEnd, arcCenter, textPos);

    ASSERT_FALSE(success);
}

TEST_CASE(measurement_geometry_builds_dihedral_geometry) {
    const float point1[3] = { 1.0f, 0.0f, 0.0f };
    const float point2[3] = { 0.0f, 0.0f, 0.0f };
    const float point3[3] = { 0.0f, 1.0f, 0.0f };
    const float point4[3] = { 0.0f, 1.0f, 1.0f };
    atoms::domain::MeasurementDihedralGeometry geometry;

    const bool success = atoms::domain::computeDihedralMeasurementGeometry(
        point1, point2, point3, point4, geometry);

    ASSERT_TRUE(success);
    ASSERT_NEAR(geometry.dihedralDeg, -90.0, 1e-6);
    ASSERT_EQ(geometry.helperArcPoints.size(), static_cast<size_t>(49));
}

TEST_CASE(measurement_geometry_rejects_degenerate_dihedral) {
    const float point1[3] = { 0.0f, 0.0f, 0.0f };
    const float point2[3] = { 1.0f, 0.0f, 0.0f };
    const float point3[3] = { 2.0f, 0.0f, 0.0f };
    const float point4[3] = { 3.0f, 0.0f, 0.0f };
    atoms::domain::MeasurementDihedralGeometry geometry;

    const bool success = atoms::domain::computeDihedralMeasurementGeometry(
        point1, point2, point3, point4, geometry);

    ASSERT_FALSE(success);
}
