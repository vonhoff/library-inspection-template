# This CMakeLists is intended to be included.
# It should not be loaded as a standalone project.

enable_testing()

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

target_include_directories(LibraryTests PRIVATE ${CLOVE_UNIT_SOURCE_DIR} ../include vendor)
target_link_libraries(LibraryTests PRIVATE Library)
