cmake_minimum_required(VERSION 3.10)
project(JsonDiffPatch)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(include)
include_directories(thirdparty)

# Create library
add_library(JsonDiffPatch 
    src/JsonDiffPatch.cpp
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
)

# Set output name for shared library
set_target_properties(JsonDiffPatchShared PROPERTIES OUTPUT_NAME JsonDiffPatch)