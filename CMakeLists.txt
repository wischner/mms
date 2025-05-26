cmake_minimum_required(VERSION 3.15)
project(mms VERSION 1.0.0 LANGUAGES CXX)

# --- Global output directories --------------------------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

# --- Build options --------------------------------------------------
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(BUILD_TESTS      "Build and run tests"  ON)

# --- C++ standard ---------------------------------------------------
set(CMAKE_CXX_STANDARD     20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS   OFF)

# --- Debug ----------------------------------------------------------
set(CMAKE_BUILD_TYPE Debug)

# --- Subdirectories -------------------------------------------------
add_subdirectory(src)

if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

# --- Installation ---------------------------------------------------
install(
  DIRECTORY include/mms
  DESTINATION include
)

install(
  TARGETS mms
  EXPORT  mmsTargets
  RUNTIME DESTINATION bin
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
)

install(
  EXPORT mmsTargets
  FILE   mmsTargets.cmake
  NAMESPACE mms::
  DESTINATION lib/cmake/mms
)
