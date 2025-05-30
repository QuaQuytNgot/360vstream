cmake_minimum_required(VERSION 3.21)

include(cmake/prelude.cmake)

project(
    vstream
    VERSION 0.1.0
    DESCRIPTION "Short description"
    HOMEPAGE_URL "https://example.com/"
    LANGUAGES C
)

include(cmake/variables.cmake)

# ---- Declare library ----

file(GLOB VSTREAM_SHARED_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source/*)

add_library(
    vstream_vstream
    ${VSTREAM_SHARED_SOURCE}
)
add_library(vstream::vstream ALIAS vstream_vstream)

# find_package(PkgConfig REQUIRED)
# pkg_check_modules(curl REQUIRED IMPORTED_TARGET libcurl)

target_link_libraries(
    vstream_vstream PRIVATE
    /home/ohmypc/curl/build/lib/libcurl.so
    m
)

include(GenerateExportHeader)
generate_export_header(
    vstream_vstream
    BASE_NAME vstream
    EXPORT_FILE_NAME export/vstream/vstream_export.h
)

if(NOT BUILD_SHARED_LIBS)
  target_compile_definitions(vstream_vstream PUBLIC VSTREAM_STATIC_DEFINE)
endif()

set_target_properties(
    vstream_vstream PROPERTIES
    C_VISIBILITY_PRESET hidden
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
    EXPORT_NAME vstream
    OUTPUT_NAME vstream
)

target_include_directories(
    vstream_vstream ${warning_guard}
    PUBLIC
    "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_include_directories(
    vstream_vstream SYSTEM
    PUBLIC
    "\$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/export>"
)

target_compile_features(vstream_vstream PUBLIC c_std_17)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Examples ----

if(PROJECT_IS_TOP_LEVEL)
  option(BUILD_EXAMPLES "Build examples tree." "${vstream_DEVELOPER_MODE}")
  if(BUILD_EXAMPLES)
    add_subdirectory(example)
  endif()
endif()

# ---- Developer mode ----

if(NOT vstream_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of vstream"
  )
endif()

include(cmake/dev-mode.cmake)
