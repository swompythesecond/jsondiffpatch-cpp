cmake_minimum_required(VERSION 3.10)
project(JsonDiffPatch)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(include)
include_directories(thirdparty)

# Create library with proper headers
add_library(JsonDiffPatch STATIC
    src/JsonDiffPatch.cpp
    include/JsonDiffPatch/JsonDiffPatch.h
)

# Set include directories for the library
target_include_directories(JsonDiffPatch PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty
)

# Create example executable
add_executable(basic_example 
    examples/basic_example.cpp
)

# Link example with library
target_link_libraries(basic_example JsonDiffPatch)

# Optional: Create shared library
add_library(JsonDiffPatchShared SHARED
    src/JsonDiffPatch.cpp
    include/JsonDiffPatch/JsonDiffPatch.h
)

# Set output name for shared library
set_target_properties(JsonDiffPatchShared PROPERTIES OUTPUT_NAME JsonDiffPatch)

# Set include directories for shared library
target_include_directories(JsonDiffPatchShared PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty
)

# Create test executable (only compile run_all_tests.cpp which includes the others)
add_executable(run_tests 
    tests/run_all_tests.cpp
)

# Link test with library
target_link_libraries(run_tests JsonDiffPatch)