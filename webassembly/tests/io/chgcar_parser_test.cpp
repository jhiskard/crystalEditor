#include "../test_harness.h"
#include "../test_support.h"
#include "../../src/atoms/infrastructure/chgcar_parser.h"

#include <string>

using atoms::infrastructure::ChgcarParser;

TEST_CASE(chgcar_parser_parses_valid_fixture) {
    const auto contents = wb::tests::readFixtureText("io/sample_chgcar.vasp");
    const auto result = ChgcarParser::parseFromString(contents);

    ASSERT_TRUE(result.success);
    ASSERT_EQ(result.systemName, std::string("Sample CHGCAR"));
    ASSERT_EQ(result.elements.size(), static_cast<size_t>(2));
    ASSERT_EQ(result.elements[0], std::string("Si"));
    ASSERT_EQ(result.elements[1], std::string("O"));
    ASSERT_EQ(result.atomCounts.size(), static_cast<size_t>(2));
    ASSERT_EQ(result.atomCounts[0], 1);
    ASSERT_EQ(result.atomCounts[1], 1);
    ASSERT_EQ(result.positions.size(), static_cast<size_t>(2));
    ASSERT_EQ(result.gridShape[0], 2);
    ASSERT_EQ(result.gridShape[1], 2);
    ASSERT_EQ(result.gridShape[2], 2);
    ASSERT_EQ(result.density.size(), static_cast<size_t>(8));
    ASSERT_NEAR(result.minValue, 0.0, 1e-6);
    ASSERT_NEAR(result.maxValue, 0.7, 1e-6);
}

TEST_CASE(chgcar_parser_rejects_invalid_content) {
    const auto result = ChgcarParser::parseFromString("invalid chgcar content");

    ASSERT_FALSE(result.success);
    ASSERT_FALSE(result.errorMessage.empty());
}
