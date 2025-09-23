cmake_minimum_required(VERSION 3.10)
project(JsonDiffPatch)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(include)
include_directories(thirdparty)

# Create static library
add_library(JsonDiffPatch STATIC
    src/JsonDiffPatch.cpp
    include/JsonDiffPatch/JsonDiffPatch.h
)

# Set include directories for the library
target_include_directories(JsonDiffPatch PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty
)

# Create DLL for GameMaker and other external applications
add_library(JsonDiffPatchDLL SHARED
    src/JsonDiffPatch.cpp
    include/JsonDiffPatch/JsonDiffPatch.h
)

# Set output name for DLL
set_target_properties(JsonDiffPatchDLL PROPERTIES 
    OUTPUT_NAME JsonDiffPatch
    PREFIX ""
)

# Define export macro for DLL
target_compile_definitions(JsonDiffPatchDLL PRIVATE JSONDIFFPATCH_EXPORTS)

# Set include directories for DLL
target_include_directories(JsonDiffPatchDLL PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty
)

# On Windows, use .def file for exports
if(WIN32)
    set_target_properties(JsonDiffPatchDLL PROPERTIES 
        LINK_FLAGS "/DEF:${CMAKE_CURRENT_SOURCE_DIR}/JsonDiffPatch.def"
    )
endif()

# Create DLL test executable
add_executable(test_dll test_dll.cpp)

# Create example executable
add_executable(basic_example 
    examples/basic_example.cpp
)

# Link example with library
target_link_libraries(basic_example JsonDiffPatch)

# Create test executable (only compile run_all_tests.cpp which includes the others)
add_executable(run_tests 
    tests/run_all_tests.cpp
)

# Link test with library
target_link_libraries(run_tests JsonDiffPatch)

# Install targets
install(TARGETS JsonDiffPatchDLL
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(TARGETS JsonDiffPatch
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)