cmake_minimum_required(VERSION 3.15)
project(mms_tests NONE)

# Fetch GoogleTest automatically
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.13.0
)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Build the test executable (links against the mms library)
add_executable(test-mms
    main.cpp
    test-bookmark.cpp
    test-postrack.cpp
    test-file.cpp
    test-streambuf.cpp
    test-istream.cpp
)
target_include_directories(test-mms
    PRIVATE
      ${CMAKE_SOURCE_DIR}/include   # public headers for mms
)
target_link_libraries(test-mms
    PRIVATE
      mms              # your main library
      GTest::gtest_main
)

# Copy test data into the runtime output directory (e.g. bin/data)
file(COPY
    ${CMAKE_CURRENT_SOURCE_DIR}/data
    DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

# Register the test; it will run in the default working directory (build root)
enable_testing()
add_test(NAME mms_basic_tests COMMAND test-mms)
