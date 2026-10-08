# Require a reasonably modern CMake version so the project can use the
# features used below.
cmake_minimum_required(VERSION 3.16)

# Name the project and declare that it uses the C++ language.
project(CANSPI_Library LANGUAGES CXX)

# Use the C++17 language standard for the build.
set(CMAKE_CXX_STANDARD 17)

# Require that C++17 is available; fail if the compiler does not support it.
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Disable compiler-specific language extensions to keep the build more portable.
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable CTest support so tests can be registered for this project.
include(CTest)
find_package(GTest REQUIRED)

# Collect all C++ source files under src/ into a list. The CONFIGURE_DEPENDS
# option causes CMake to re-run the glob when files change.
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
    *.cpp
)
list(FILTER PROJECT_SOURCES EXCLUDE REGEX ".*/build/.*")

# Define the library target named CANSPI using the discovered source files.
add_library(CANSPI SHARED ${PROJECT_SOURCES})
set_target_properties(CANSPI PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Performance: Hide internal symbols from the shared binary interface
# set_target_properties(CANSPI PROPERTIES
#     C_VISIBILITY_PRESET hidden
#     CXX_VISIBILITY_PRESET hidden
#     VISIBILITY_INLINES_HIDDEN ON
# )

# Make the project root, src/, and lib/ directories available to the compiler
# so headers can be included from those locations.
target_include_directories(CANSPI PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_options(CANSPI PRIVATE -Wall -Wextra -Wpedantic)
target_compile_options(CANSPI PUBLIC -O3) # Enable aggressive optimizations (vectorization, inlining)

# Determine 32-bit or 64-bit ARM system
if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64" OR CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(STATUS "Compiling CANSPI 64-bit")
else()
    target_compile_options(CANSPI PRIVATE
        -march=armv6zk      # Target the exact CPU architecture of the Pi Zero
        -mcpu=arm1176jzf-s  # Optimize instruction scheduling for this specific core
        -mfpu=vfp           # Explicitly specify the standard VFP unit
    )

    target_compile_options(CANSPI PUBLIC -mfloat-abi=hard) # Ensure the compiler uses the hardware VFP11 coprocessor

    message(STATUS "Compiling CANSPI 32-bit")
endif()

# Check if the user explicitly requested a Debug build via -DCMAKE_BUILD_TYPE=Debug
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Debug build detected. Adding full debug symbols.")
else()
    message(STATUS "Standard build detected.")
endif()

if(BUILD_TESTING)
    # Load CMake module for GoogleTest
    include(GoogleTest)

    # Find all .cc files inside the tests/ directory and store them in the TEST_FILES variable
    file(GLOB TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cc")

    # Discover test files
    foreach(TEST_FILE_PATH ${TEST_FILES})
        # Get filename without extension
        get_filename_component(TEST_NAME ${TEST_FILE_PATH} NAME_WE)

        # Define executable for this specific test file
        add_executable(${TEST_NAME} ${TEST_FILE_PATH})

        # Link the executable against your core library
        target_link_libraries(${TEST_NAME} PRIVATE CANSPI GTest::gtest GTest::gtest_main)

        # Discover and register tests in the test executable
        gtest_discover_tests(${TEST_NAME})
    endforeach()
endif()
