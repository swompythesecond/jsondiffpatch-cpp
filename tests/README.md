# JsonDiffPatch Tests

This directory contains comprehensive unit tests for the JsonDiffPatch C++ library.

## Test Structure

- **`test_framework.h`** - Simple lightweight testing framework with assertion macros
- **`test_jsondiffpatch.cpp`** - Core functionality tests (diff, patch, unpatch)
- **`test_edge_cases.cpp`** - Edge cases, performance tests, and advanced scenarios
- **`run_all_tests.cpp`** - Main test runner that includes all test files

## Test Coverage

### Core Functionality Tests
- ✅ Basic object diff/patch/unpatch
- ✅ Object addition and deletion
- ✅ Array diff/patch/unpatch
- ✅ Array addition and deletion
- ✅ String diffing
- ✅ Nested object operations
- ✅ Mixed type arrays
- ✅ C API functionality

### Edge Cases and Advanced Tests
- ✅ Text diff algorithms (SimpleTextDiff)
- ✅ Text encoding/decoding
- ✅ Large array operations (100+ elements)
- ✅ Deeply nested objects (4+ levels)
- ✅ Mixed type arrays (numbers, strings, booleans, objects)
- ✅ Invalid JSON handling in C API
- ✅ Null pointer handling
- ✅ Complex array LCS (Longest Common Subsequence)
- ✅ Array reordering
- ✅ Performance with large objects (1000+ properties)
- ✅ Special characters in strings
- ✅ Options configuration testing

## Running Tests

### Using CMake
```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
./run_tests                    # Linux/Mac
run_tests.exe                  # Windows
```

### Using Make
```bash
make all                       # Build library
make test                      # Build tests
./run_tests                    # Run tests
```

### Using Scripts
```bash
# Linux/Mac
./run_tests.sh

# Windows
run_tests.bat
```

### Using CTest (CMake)
```bash
cd build
ctest --verbose
```

## Test Output

The tests will output:
- Individual test results (PASSED/FAILED)
- Summary of total tests run
- Number of passed/failed tests
- Exit code (0 for success, non-zero for failures)

Example output:
```
Running 25 tests...

Running: BasicObjectDiff ... PASSED
Running: ObjectAddition ... PASSED
Running: ObjectDeletion ... PASSED
...

=== Test Results ===
Passed: 25
Failed: 0
Total:  25
All tests passed! ✓
```

## Adding New Tests

To add new tests, use the `TEST(testName)` macro:

```cpp
TEST(MyNewTest) {
    JsonDiffPatch jdp;
    
    json left = {{"test", true}};
    json right = {{"test", false}};
    
    json diff = jdp.Diff(left, right);
    
    ASSERT_FALSE(diff.is_null());
    ASSERT_TRUE(diff.contains("test"));
}
```

Available assertion macros:
- `ASSERT_TRUE(condition)`
- `ASSERT_FALSE(condition)`
- `ASSERT_EQ(expected, actual)`
- `ASSERT_NE(expected, actual)`

## Memory Testing

For memory leak detection, run tests with valgrind (Linux) or similar tools:

```bash
# Linux with valgrind
valgrind --leak-check=full ./run_tests

# Windows with Application Verifier or similar tools
```

## Performance Profiling

The tests include performance tests with large datasets. To profile:

```bash
# Linux with perf
perf record ./run_tests
perf report

# Or use other profiling tools
```