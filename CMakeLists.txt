cmake_minimum_required(VERSION 3.25)
project(Library C)

# Set architecture-specific output directory
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(ARCHITECTURE "x64")
else ()
    set(ARCHITECTURE "x86")
endif ()
set(OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/$<CONFIGURATION>-${ARCHITECTURE}/${PROJECT_NAME}")

# Enable position-independent code with warnings
add_compile_options(-fPIC -Wall -Wextra -Wpedantic)

# Set optimization flags for Release builds
if (CMAKE_BUILD_TYPE MATCHES Release)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden -O3")
endif ()

# Create Library
file(GLOB SOURCES "./src/*.c")
file(GLOB HEADERS "./src/*.h" "./include/*.h")
add_library(Library SHARED ${HEADERS} ${SOURCES})
target_compile_definitions(Library PRIVATE LIB_EXPORT)
target_include_directories(Library PRIVATE include)

set_target_properties(
        Library PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}"
        RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}"
        C_STANDARD 11
        C_STANDARD_REQUIRED ON
)

add_subdirectory(tests)
