#pragma once

#include <iostream>
#include <string>
#include <functional>
#include <vector>
#include <sstream>

// Simple test framework
class TestRunner {
private:
    struct Test {
        std::string name;
        std::function<void()> testFunc;
    };
    
    std::vector<Test> tests;
    int passed = 0;
    int failed = 0;
    
public:
    void addTest(const std::string& name, std::function<void()> testFunc) {
        tests.push_back({name, testFunc});
    }
    
    void runAll() {
        std::cout << "Running " << tests.size() << " tests...\n" << std::endl;
        
        for (const auto& test : tests) {
            try {
                std::cout << "Running: " << test.name << " ... ";
                test.testFunc();
                std::cout << "PASSED" << std::endl;
                passed++;
            } catch (const std::exception& e) {
                std::cout << "FAILED: " << e.what() << std::endl;
                failed++;
            }
        }
        
        std::cout << "\n=== Test Results ===" << std::endl;
        std::cout << "Passed: " << passed << std::endl;
        std::cout << "Failed: " << failed << std::endl;
        std::cout << "Total:  " << (passed + failed) << std::endl;
        
        if (failed == 0) {
            std::cout << "All tests passed! ✓" << std::endl;
        } else {
            std::cout << "Some tests failed! ✗" << std::endl;
        }
    }
    
    int getFailedCount() const { return failed; }
};

// Test assertion macros
#define ASSERT_TRUE(condition) \
    if (!(condition)) { \
        std::ostringstream oss; \
        oss << "Assertion failed: " << #condition << " at line " << __LINE__; \
        throw std::runtime_error(oss.str()); \
    }

#define ASSERT_FALSE(condition) \
    if (condition) { \
        std::ostringstream oss; \
        oss << "Assertion failed: " << #condition << " should be false at line " << __LINE__; \
        throw std::runtime_error(oss.str()); \
    }

#define ASSERT_EQ(expected, actual) \
    if ((expected) != (actual)) { \
        std::ostringstream oss; \
        oss << "Assertion failed: expected " << (expected) << ", got " << (actual) << " at line " << __LINE__; \
        throw std::runtime_error(oss.str()); \
    }

#define ASSERT_NE(expected, actual) \
    if ((expected) == (actual)) { \
        std::ostringstream oss; \
        oss << "Assertion failed: " << (expected) << " should not equal " << (actual) << " at line " << __LINE__; \
        throw std::runtime_error(oss.str()); \
    }

#define TEST(testName) \
    void test_##testName(); \
    struct TestRegistrar_##testName { \
        TestRegistrar_##testName() { \
            extern TestRunner globalTestRunner; \
            globalTestRunner.addTest(#testName, test_##testName); \
        } \
    }; \
    static TestRegistrar_##testName registrar_##testName; \
    void test_##testName()