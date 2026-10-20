# Project-wide compiler settings, build options, and output layout.
if(APPLE)
  enable_language(OBJCXX)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(YH_ENABLE_PCH "Use precompiled headers for C++ targets" ON)
option(YH_ENABLE_PACKAGING "Configure the native release installer or archive" OFF)

include(CTest)
include(GNUInstallDirs)

# DLLs and executables share a directory on Windows, including test programs.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
if(APPLE)
  set(CMAKE_INSTALL_RPATH "@loader_path;@loader_path/../${CMAKE_INSTALL_LIBDIR}")
elseif(UNIX)
  set(CMAKE_INSTALL_RPATH "$ORIGIN;$ORIGIN/../${CMAKE_INSTALL_LIBDIR}")
endif()

set(_desktop_default OFF)
if(APPLE OR WIN32)
  set(_desktop_default ON)
endif()
option(BUILD_DESKTOP_APP "Build the Windows or macOS desktop application" ${_desktop_default})

option(BUILD_CLI_APP "Build the console CLI" ON)
option(BUILD_REST_APP "Build the headless REST worker" ON)
option(BUILD_SHORTCUTS_APP "Build the native global shortcut worker" ${BUILD_DESKTOP_APP})
if(BUILD_SHORTCUTS_APP AND NOT (APPLE OR WIN32))
  message(FATAL_ERROR "Global shortcuts are supported on Windows and macOS only")
endif()
option(YH_BUILD_REST_ADAPTER "Build the reusable REST adapter" ${BUILD_REST_APP})
if(BUILD_REST_APP AND NOT YH_BUILD_REST_ADAPTER)
  message(FATAL_ERROR "BUILD_REST_APP requires YH_BUILD_REST_ADAPTER")
endif()

if(BUILD_DESKTOP_APP AND NOT (APPLE OR WIN32))
  message(FATAL_ERROR "The desktop application is supported on Windows and macOS only")
endif()
