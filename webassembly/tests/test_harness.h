#pragma once

#include <cmath>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace wb::tests {

using TestFunction = void(*)();

struct TestCase {
    const char* name;
    TestFunction function;
};

class TestFailure : public std::runtime_error {
public:
    TestFailure(const char* file, int line, const std::string& message)
        : std::runtime_error(buildMessage(file, line, message)) {}

private:
    static std::string buildMessage(const char* file, int line, const std::string& message) {
        std::ostringstream oss;
        oss << file << ":" << line << ": " << message;
        return oss.str();
    }
};

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> tests;
    return tests;
}

struct Registrar {
    Registrar(const char* name, TestFunction function) {
        registry().push_back({ name, function });
    }
};

template<typename Lhs, typename Rhs>
inline void assertEqual(
    const Lhs& lhs,
    const Rhs& rhs,
    const char* lhsExpr,
    const char* rhsExpr,
    const char* file,
    int line) {
    if (!(lhs == rhs)) {
        std::ostringstream oss;
        oss << "expected " << lhsExpr << " == " << rhsExpr << ", got [" << lhs << "] vs [" << rhs << "]";
        throw TestFailure(file, line, oss.str());
    }
}

inline void assertNear(
    double lhs,
    double rhs,
    double tolerance,
    const char* lhsExpr,
    const char* rhsExpr,
    const char* toleranceExpr,
    const char* file,
    int line) {
    if (std::fabs(lhs - rhs) > tolerance) {
        std::ostringstream oss;
        oss << "expected " << lhsExpr << " ~= " << rhsExpr
            << " within " << toleranceExpr
            << ", got [" << lhs << "] vs [" << rhs << "]";
        throw TestFailure(file, line, oss.str());
    }
}

} // namespace wb::tests

#define TEST_CASE(name) \
    static void name(); \
    static ::wb::tests::Registrar registrar_##name(#name, &name); \
    static void name()

#define ASSERT_TRUE(expr) \
    do { \
        if (!(expr)) { \
            throw ::wb::tests::TestFailure(__FILE__, __LINE__, "expected true: " #expr); \
        } \
    } while (0)

#define ASSERT_FALSE(expr) \
    do { \
        if (expr) { \
            throw ::wb::tests::TestFailure(__FILE__, __LINE__, "expected false: " #expr); \
        } \
    } while (0)

#define ASSERT_EQ(lhs, rhs) \
    ::wb::tests::assertEqual((lhs), (rhs), #lhs, #rhs, __FILE__, __LINE__)

#define ASSERT_NEAR(lhs, rhs, tolerance) \
    ::wb::tests::assertNear((lhs), (rhs), (tolerance), #lhs, #rhs, #tolerance, __FILE__, __LINE__)
