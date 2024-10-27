cmake_minimum_required(VERSION 3.22.1) # my version

project(netcpp VERSION 0.0.0)

set(LINK_LIBS)
set(SRC_FILES src/socket.cpp)


include(GNUInstallDirs)

add_library(netcpp SHARED ${SRC_FILES})
add_library(netcpp::netcpp ALIAS netcpp)

target_link_libraries(netcpp PRIVATE ${LINK_LIBS})

target_include_directories(
  netcpp
  PUBLIC
  $<INSTALL_INTERFACE:include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(TARGETS netcpp
  EXPORT netcppTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Include the CMakePackageConfigHelpers module
include(CMakePackageConfigHelpers)

# Set some properties for your library target
set_property(TARGET netcpp PROPERTY VERSION ${PROJECT_VERSION})
set_property(TARGET netcpp PROPERTY SOVERSION 1)
set_property(TARGET netcpp PROPERTY
  INTERFACE_netcpp_MAJOR_VERSION 1)
set_property(TARGET netcpp APPEND PROPERTY
  COMPATIBLE_INTERFACE_STRING netcpp_MAJOR_VERSION
)

# Generate the version file for the config file
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/cmake/netcppConfigVersion.cmake"
  VERSION "${PROJECT_VERSION}"
  COMPATIBILITY AnyNewerVersion
)

# Create the config file
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/netcppConfig.cmake
  "${CMAKE_CURRENT_BINARY_DIR}/cmake/netcppConfig.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/netcpp
)

# Install the config and version files
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/cmake/netcppConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake/netcppConfigVersion.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/netcpp
)

# Export the targets for the build tree
export(EXPORT netcppTargets
  FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/netcppTargets.cmake"
  NAMESPACE netcpp::
)

# Install the export set for the install tree
install(EXPORT netcppTargets
  FILE netcppTargets.cmake
  NAMESPACE netcpp::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/netcpp
)


