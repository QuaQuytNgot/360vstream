if(PROJECT_IS_TOP_LEVEL)
  set(
      CMAKE_INSTALL_INCLUDEDIR "include/vstream-${PROJECT_VERSION}"
      CACHE STRING ""
  )
  set_property(CACHE CMAKE_INSTALL_INCLUDEDIR PROPERTY TYPE PATH)
endif()

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# find_package(<package>) call for consumers to find this project
set(package vstream)

install(
    DIRECTORY
    include/
    "${PROJECT_BINARY_DIR}/export/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    COMPONENT vstream_Development
)

install(
    TARGETS vstream_vstream
    EXPORT vstreamTargets
    RUNTIME #
    COMPONENT vstream_Runtime
    LIBRARY #
    COMPONENT vstream_Runtime
    NAMELINK_COMPONENT vstream_Development
    ARCHIVE #
    COMPONENT vstream_Development
    INCLUDES #
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

write_basic_package_version_file(
    "${package}ConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
)

# Allow package maintainers to freely override the path for the configs
set(
    vstream_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/${package}"
    CACHE STRING "CMake package config location relative to the install prefix"
)
set_property(CACHE vstream_INSTALL_CMAKEDIR PROPERTY TYPE PATH)
mark_as_advanced(vstream_INSTALL_CMAKEDIR)

install(
    FILES cmake/install-config.cmake
    DESTINATION "${vstream_INSTALL_CMAKEDIR}"
    RENAME "${package}Config.cmake"
    COMPONENT vstream_Development
)

install(
    FILES "${PROJECT_BINARY_DIR}/${package}ConfigVersion.cmake"
    DESTINATION "${vstream_INSTALL_CMAKEDIR}"
    COMPONENT vstream_Development
)

install(
    EXPORT vstreamTargets
    NAMESPACE vstream::
    DESTINATION "${vstream_INSTALL_CMAKEDIR}"
    COMPONENT vstream_Development
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
