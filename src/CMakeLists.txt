# set source files
set (server-src-files
     provider.c)

set (client-src-files
     client.c)

set (admin-src-files
     admin.c)

set (dummy-src-files
     dummy/dummy-backend.c)

set (bedrock-module-src-files
     bedrock-module.c)

# load package helper for generating cmake CONFIG packages
include (CMakePackageConfigHelpers)

# where to install files for "find_package"
set (YP-pkg "share/cmake/YP")

# set version fo shared library
set (YP-vers "${YP_VERSION_MAJOR}.${YP_VERSION_MINOR}")

# server library
add_library (YP-server ${server-src-files} ${dummy-src-files})
target_link_libraries (YP-server
    PUBLIC PkgConfig::margo PkgConfig::uuid
    PRIVATE coverage_config PkgConfig::json-c)
target_include_directories (YP-server PUBLIC $<INSTALL_INTERFACE:include>)
target_include_directories (YP-server BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>)
target_include_directories (YP-server BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
set_target_properties (YP-server
    PROPERTIES VERSION ${YP_VERSION}
    SOVERSION ${YP_VERSION_MAJOR})

# client library
add_library (YP-client ${client-src-files})
target_link_libraries (YP-client
    PUBLIC PkgConfig::margo PkgConfig::uuid
    PRIVATE coverage_config)
target_include_directories (YP-client PUBLIC $<INSTALL_INTERFACE:include>)
target_include_directories (YP-client BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>)
target_include_directories (YP-client BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
set_target_properties (YP-client
    PROPERTIES VERSION ${YP_VERSION}
    SOVERSION ${YP_VERSION_MAJOR})

# admin library
add_library (YP-admin ${admin-src-files})
target_link_libraries (YP-admin
    PUBLIC PkgConfig::margo PkgConfig::uuid
    PRIVATE coverage_config)
target_include_directories (YP-admin PUBLIC $<INSTALL_INTERFACE:include>)
target_include_directories (YP-admin BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>)
target_include_directories (YP-admin BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
set_target_properties (YP-admin
    PROPERTIES VERSION ${YP_VERSION}
    SOVERSION ${YP_VERSION_MAJOR})

if (${ENABLE_BEDROCK})
# bedrock-module library
add_library (YP-bedrock-module ${bedrock-module-src-files})
target_link_libraries (YP-bedrock-module YP-server YP-admin YP-client bedrock-server)
target_include_directories (YP-bedrock-module PUBLIC $<INSTALL_INTERFACE:include>)
target_include_directories (YP-bedrock-module BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>)
target_include_directories (YP-bedrock-module BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
set_target_properties (YP-bedrock-module
    PROPERTIES VERSION ${YP_VERSION}
    SOVERSION ${YP_VERSION_MAJOR})
endif ()

# installation stuff (packaging and install commands)
write_basic_package_version_file (
    "YP-config-version.cmake"
    VERSION ${YP_VERSION}
    COMPATIBILITY AnyNewerVersion)

# generate our config file for find_package()
set (INCLUDE_INSTALL_DIR include/)
set (LIB_INSTALL_DIR lib/)
configure_package_config_file (YP-config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/YP-config.cmake
  INSTALL_DESTINATION ${LIB_INSTALL_DIR}/YP/cmake
  PATH_VARS INCLUDE_INSTALL_DIR)

# some bits for the pkg-config file
set (DEST_DIR "${CMAKE_INSTALL_PREFIX}")
set (SERVER_PRIVATE_LIBS "-lYP-server")
set (CLIENT_PRIVATE_LIBS "-lYP-client")
set (ADMIN_PRIVATE_LIBS  "-lYP-admin")
configure_file ("YP-server.pc.in" "YP-server.pc" @ONLY)
configure_file ("YP-client.pc.in" "YP-client.pc" @ONLY)
configure_file ("YP-admin.pc.in"  "YP-admin.pc"  @ONLY)

# configure config.h
configure_file ("config.h.in" "config.h" @ONLY)

# "make install" rules
install (TARGETS YP-server YP-client YP-admin
         EXPORT YP-targets
         ARCHIVE DESTINATION lib
         LIBRARY DESTINATION lib)
if (${ENABLE_BEDROCK})
    install (TARGETS YP-bedrock-module
             ARCHIVE DESTINATION lib
             LIBRARY DESTINATION lib)
endif ()
install (EXPORT YP-targets
         DESTINATION ${YP-pkg}
         FILE "YP-targets.cmake")
install (FILES "${CMAKE_CURRENT_BINARY_DIR}/YP-config.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/YP-config-version.cmake"
         DESTINATION ${YP-pkg} )
install (DIRECTORY ../include/YP
         DESTINATION include
         FILES_MATCHING PATTERN "*.h")
install (FILES "${CMAKE_CURRENT_BINARY_DIR}/YP-server.pc"
               "${CMAKE_CURRENT_BINARY_DIR}/YP-client.pc"
               "${CMAKE_CURRENT_BINARY_DIR}/YP-admin.pc"
         DESTINATION "lib/pkgconfig/")
