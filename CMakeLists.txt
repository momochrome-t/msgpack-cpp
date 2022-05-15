CMAKE_MINIMUM_REQUIRED (VERSION 3.16 FATAL_ERROR)

PROJECT (msgpack LANGUAGES CXX)

SET (CMAKE_CXX_STANDARD 20)
SET (CMAKE_CXX_STANDARD_REQUIRED True)

ADD_LIBRARY (msgpack-cpp INTERFACE)

TARGET_INCLUDE_DIRECTORIES (msgpack-cpp
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

add_subdirectory(sample)

include(GNUInstallDirs)

install(TARGETS msgpack-cpp
        EXPORT msgpack-cpp-targets
        COMPONENT msgpack-cpp
        INCLUDES DESTINATION "${cmake_install_includedir}"
)
install(DIRECTORY include/
        DESTINATION include
        COMPONENT msgpack-cpp
)

include(CMakePackageConfigHelpers)

set(cmake_config_path "${cmake_install_libdir}/cmake/msgpack-cpp")

CONFIGURE_PACKAGE_CONFIG_FILE (
    msgpack-config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/msgpack-config.cmake"
    INSTALL_DESTINATION "${cmake_config_path}"
)

INSTALL (
    EXPORT msgpack-cpp-targets
    FILE msgpack-cpp-targets.cmake
    DESTINATION "${cmake_config_path}"
    COMPONENT msgpack-cpp
)