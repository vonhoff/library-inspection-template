cmake_minimum_required(VERSION 3.15)

# Project configuration
set(PROJECT_NAME Library)
set(TEST_PROJECT_NAME LibraryTests)

project(${PROJECT_NAME})
set(CMAKE_C_STANDARD 11)

# Append binary directory to module and prefix paths.
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

# Download conan.cmake if it doesn't exist.
if (NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/develop/conan.cmake" "${CMAKE_BINARY_DIR}/conan.cmake" TLS_VERIFY ON)
endif ()

include(${CMAKE_BINARY_DIR}/conan.cmake)

# Configure and install Conan dependencies.
conan_cmake_configure(REQUIRES clove-unit/2.4.1 GENERATORS CMakeToolchain CMakeDeps)
conan_cmake_autodetect(settings)
conan_cmake_install(PATH_OR_REFERENCE . BUILD missing REMOTE conancenter SETTINGS ${settings})

find_package(clove-unit REQUIRED)

if (MSVC)
    # For MSVC, enable level 4 warnings.
    add_compile_options(/W4)
else ()
    # For other compilers, enable position-independent code and all warnings.
    add_compile_options(-fPIC -Wall -Wextra -Wpedantic)
endif ()

# Set optimization flags.
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden -O3")

# Add source files to library
file(GLOB_RECURSE SOURCES "src/*.c")
file(GLOB_RECURSE HEADERS "src/*.h" "include/*.h")
add_library(${PROJECT_NAME} SHARED ${HEADERS} ${SOURCES})
target_compile_definitions(${PROJECT_NAME} PRIVATE LIB_EXPORT)
target_include_directories(${PROJECT_NAME} PRIVATE include)

# Enable testing and add source files to test project.
enable_testing()
file(GLOB_RECURSE TEST_SOURCES "test/*.c")
file(GLOB_RECURSE TEST_HEADERS "test/*.h")
add_executable(${TEST_PROJECT_NAME} ${TEST_HEADERS} ${TEST_SOURCES})

# Set compile options, definitions, and properties for the test project.
target_compile_definitions(${TEST_PROJECT_NAME} PRIVATE PROJECT_ROOT="${CMAKE_SOURCE_DIR}")
target_include_directories(${TEST_PROJECT_NAME} PRIVATE include)
target_link_libraries(${TEST_PROJECT_NAME} PRIVATE ${PROJECT_NAME} clove-unit::clove-unit)

# Disable all warnings for the test project.
target_compile_options(${TEST_PROJECT_NAME} PRIVATE -w)
