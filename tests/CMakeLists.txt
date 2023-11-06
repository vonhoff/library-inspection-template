# This CMakeLists is intended to be included in the main project.
# It should not be loaded as a standalone project.

enable_testing()
include(FetchContent)

set(CLOVE_UNIT_REPO "https://github.com/fdefelici/clove-unit")
set(CLOVE_UNIT_FILE "clove-unit.h")
set(VENDOR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor)

# Fetch the content from the clove-unit repository
FetchContent_Declare(
        clove_unit
        GIT_REPOSITORY ${CLOVE_UNIT_REPO}
        GIT_TAG master
)

FetchContent_GetProperties(clove_unit)
if (NOT clove_unit_POPULATED)
    FetchContent_Populate(clove_unit)

    configure_file(
            ${clove_unit_SOURCE_DIR}/${CLOVE_UNIT_FILE}
            ${VENDOR_DIR}/${CLOVE_UNIT_FILE}
            COPYONLY
    )
endif ()

# Create Tests executable
file(GLOB TEST_SOURCES "./src/*.c")
file(GLOB TEST_HEADERS "./src/*.h")
add_executable(LibraryTests ${TEST_HEADERS} ${TEST_SOURCES})
target_compile_options(LibraryTests PRIVATE -w)
target_compile_definitions(LibraryTests PRIVATE PROJECT_ROOT="${CMAKE_SOURCE_DIR}")

set_target_properties(
        LibraryTests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}"
        C_STANDARD 11
        C_STANDARD_REQUIRED ON
)

target_include_directories(LibraryTests PRIVATE ${CLOVE_UNIT_SOURCE_DIR} ${VENDOR_DIR} ../include)
target_link_libraries(LibraryTests PRIVATE Library)
