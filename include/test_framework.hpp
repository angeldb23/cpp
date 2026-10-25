#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <cmath>

namespace TestFramework {

struct TestResult {
    std::string name;
    bool passed;
    std::string message;
};

class TestRunner {
private:
    std::vector<std::pair<std::string, std::function<void(bool&)>>> tests;

public:
    static TestRunner& instance() {
        static TestRunner runner;
        return runner;
    }

    void addTest(const std::string& name, std::function<void(bool&)> func) {
        tests.push_back({name, func});
    }

    int run() {
        int passedCount = 0;
        int failedCount = 0;
        std::cout << "\n================ TEST RUNNER ================\n";
        
        for (const auto& [name, func] : tests) {
            bool testPassed = true;
            try {
                func(testPassed);
            } catch (const std::exception& e) {
                testPassed = false;
                std::cout << "  [EXCEPTION] " << name << " -> " << e.what() << "\n";
            } catch (...) {
                testPassed = false;
                std::cout << "  [EXCEPTION] " << name << " -> excepcion desconocida\n";
            }

            if (testPassed) {
                std::cout << "  [PASS] " << name << "\n";
                passedCount++;
            } else {
                std::cout << "  [FAIL] " << name << "\n";
                failedCount++;
            }
        }

        std::cout << "=============================================\n";
        std::cout << "Total: " << tests.size() 
                  << " | Pasaron: " << passedCount 
                  << " | Fallaron: " << failedCount << "\n";
        std::cout << "=============================================\n\n";

        return (failedCount == 0) ? 0 : 1;
    }
};

struct AutoRegister {
    AutoRegister(const std::string& name, std::function<void(bool&)> func) {
        TestRunner::instance().addTest(name, func);
    }
};

} // namespace TestFramework

#define TEST_CASE(name) \
    void name(bool& _test_passed); \
    static TestFramework::AutoRegister _reg_##name(#name, name); \
    void name(bool& _test_passed)

#define ASSERT_TRUE(condition) \
    do { \
        if (!(condition)) { \
            std::cout << "    Fallo en linea " << __LINE__ << ": " #condition " no es verdadero\n"; \
            _test_passed = false; \
            return; \
        } \
    } while (0)

#define ASSERT_FALSE(condition) \
    do { \
        if (condition) { \
            std::cout << "    Fallo en linea " << __LINE__ << ": " #condition " no es falso\n"; \
            _test_passed = false; \
            return; \
        } \
    } while (0)

#define ASSERT_EQ(actual, expected) \
    do { \
        auto _actual = (actual); \
        auto _expected = (expected); \
        if (_actual != _expected) { \
            std::cout << "    Fallo en linea " << __LINE__ << ": obtenido (" \
                      << _actual << ") != esperado (" << _expected << ")\n"; \
            _test_passed = false; \
            return; \
        } \
    } while (0)

#define ASSERT_NEAR(actual, expected, epsilon) \
    do { \
        auto _actual = (actual); \
        auto _expected = (expected); \
        auto _eps = (epsilon); \
        using _actual_t = std::decay_t<decltype(_actual)>; \
        using _expected_t = std::decay_t<decltype(_expected)>; \
        using _common_t = std::common_type_t<_actual_t, _expected_t>; \
        _common_t _diff = _common_t(_actual) - _common_t(_expected); \
        if (_diff < 0) _diff = -_diff; \
        if (_diff > _eps) { \
            std::cout << "    Fallo en linea " << __LINE__ << ": diferencia > " #epsilon "\n"; \
            _test_passed = false; \
            return; \
        } \
    } while (0)

#define ASSERT_NE(actual, expected) \
    do { \
        auto _actual = (actual); \
        auto _expected = (expected); \
        if (_actual == _expected) { \
            std::cout << "    Fallo en linea " << __LINE__ << ": obtenido (" \
                      << _actual << ") == inesperado (" << _expected << ")\n"; \
            _test_passed = false; \
            return; \
        } \
    } while (0)

#define ASSERT_GE(actual, expected) \
    do { \
        auto _actual = (actual); \
        auto _expected = (expected); \
        if (!(_actual >= _expected)) { \
            std::cout << "    Fallo en linea " << __LINE__ << ": obtenido (" \
                      << _actual << ") < esperado (" << _expected << ")\n"; \
            _test_passed = false; \
            return; \
        } \
    } while (0)

#define ASSERT_GT(actual, expected) \
    do { \
        auto _actual = (actual); \
        auto _expected = (expected); \
        if (!(_actual > _expected)) { \
            std::cout << "    Fallo en linea " << __LINE__ << ": obtenido (" \
                      << _actual << ") <= esperado (" << _expected << ")\n"; \
            _test_passed = false; \
            return; \
        } \
    } while (0)

#define ASSERT_LT(actual, expected) \
    do { \
        auto _actual = (actual); \
        auto _expected = (expected); \
        if (!(_actual < _expected)) { \
            std::cout << "    Fallo en linea " << __LINE__ << ": obtenido (" \
                      << _actual << ") >= esperado (" << _expected << ")\n"; \
            _test_passed = false; \
            return; \
        } \
    } while (0)

#define RUN_ALL_TESTS() TestFramework::TestRunner::instance().run()
