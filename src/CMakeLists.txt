# src/CMakeLists.txt
# Collect all sources
set(MMS_SOURCES
    bookmark.cpp
    postrack.cpp
    file.cpp
    source.cpp
)

# Create the library target
add_library(mms ${MMS_SOURCES})

# Include headers for clients
target_include_directories(mms
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Ensure Position‑Independent Code for shared builds
set_target_properties(mms PROPERTIES
  POSITION_INDEPENDENT_CODE ON
)

# (Optional) Link any required system libs here
# target_link_libraries(mms PUBLIC pthread)  # if needed
