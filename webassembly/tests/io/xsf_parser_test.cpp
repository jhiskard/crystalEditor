#include "../test_harness.h"
#include "../test_support.h"
#include "../../src/io/infrastructure/xsf_parser.h"

#include <string>

using io::infrastructure::XsfParser;

TEST_CASE(xsf_parser_parses_structure_fixture) {
    const auto result = XsfParser::parseStructureFile(
        wb::tests::fixturePath("io/sample_structure.xsf").string());

    ASSERT_TRUE(result.success);
    ASSERT_EQ(result.atoms.size(), static_cast<size_t>(2));
    ASSERT_EQ(result.atoms[0].symbol, std::string("Si"));
    ASSERT_EQ(result.atoms[1].symbol, std::string("O"));
    ASSERT_NEAR(result.cellVectors[0][0], 5.0, 1e-6);
    ASSERT_NEAR(result.cellVectors[1][1], 5.0, 1e-6);
    ASSERT_NEAR(result.cellVectors[2][2], 5.0, 1e-6);
    ASSERT_NEAR(result.atoms[1].position[0], 1.25, 1e-6);
}

TEST_CASE(xsf_parser_parses_grid_fixture) {
    const auto result = XsfParser::parseGridFile(
        wb::tests::fixturePath("io/sample_grid.xsf").string());

    ASSERT_TRUE(result.success);
    ASSERT_EQ(result.atoms.size(), static_cast<size_t>(2));
    ASSERT_EQ(result.grids.size(), static_cast<size_t>(1));
    ASSERT_TRUE(result.hasCellVectors);
    ASSERT_TRUE(result.cellVectorsConsistent);
    ASSERT_EQ(result.grids[0].high.dims[0], 2);
    ASSERT_EQ(result.grids[0].high.dims[1], 2);
    ASSERT_EQ(result.grids[0].high.dims[2], 2);
    ASSERT_EQ(result.grids[0].high.values.size(), static_cast<size_t>(8));
    ASSERT_EQ(result.grids[0].medium.values.size(), static_cast<size_t>(1));
    ASSERT_EQ(result.grids[0].low.values.size(), static_cast<size_t>(1));
    ASSERT_NEAR(result.grids[0].high.values.back(), 0.875, 1e-6);
}

TEST_CASE(xsf_parser_reports_missing_grid_block) {
    const auto result = XsfParser::parseGridFile(
        wb::tests::fixturePath("io/sample_structure.xsf").string());

    ASSERT_FALSE(result.success);
    ASSERT_EQ(result.errorMessage, std::string("No DATAGRID_3D blocks found"));
}
