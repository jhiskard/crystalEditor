#include "test_harness.h"

#include <exception>
#include <iostream>

int main() {
    const auto& tests = wb::tests::registry();
    int failed = 0;

    std::cout << "Running " << tests.size() << " Phase 6 native tests" << std::endl;
    for (const auto& test : tests) {
        try {
            test.function();
            std::cout << "[PASS] " << test.name << std::endl;
        } catch (const std::exception& e) {
            ++failed;
            std::cerr << "[FAIL] " << test.name << " - " << e.what() << std::endl;
        } catch (...) {
            ++failed;
            std::cerr << "[FAIL] " << test.name << " - unknown exception" << std::endl;
        }
    }

    if (failed > 0) {
        std::cerr << failed << " test(s) failed" << std::endl;
        return 1;
    }

    std::cout << "All native tests passed" << std::endl;
    return 0;
}
